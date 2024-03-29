/*
 * Copyright (C) 2022 Videology Inc, Inc. All Rights Reserved.
 */

#include <linux/clk.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/regmap.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/firmware.h>
#include <linux/pinctrl/consumer.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/kmod.h>
#include <media/v4l2-async.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-subdev.h>

#define MIN_HEIGHT			720
#define MIN_WIDTH			1280
#define MAX_HEIGHT			1080
#define MAX_WIDTH			1920

#define PAD_SINK			0
#define PAD_SOURCE			1
#define NUM_PADS			1

#define FIRMWARE_VERSION	0xA7
#define FIRWARE_NAME		"crosslink_lvds_A7.bit"
struct resolution {
	u16 width;
	u16 height;
	u16 framerate;
	u16 reg_val;
};

static struct resolution sensor_res_list[] = {
	// HD-SDI Single LVDS channel
	{.width = 1280, .height = 720,  .framerate = 25, .reg_val = 0x03 },    // 720p25
	{.width = 1280, .height = 720,  .framerate = 30, .reg_val = 0x02 },    // 720p30
	{.width = 1280, .height = 720,  .framerate = 50, .reg_val = 0x01 },    // 720p50
	{.width = 1280, .height = 720,  .framerate = 60, .reg_val = 0x00 },    // 720p60
	{.width = 1920, .height = 1080, .framerate = 25, .reg_val = 0x13 },    // 1080p25
	{.width = 1920, .height = 1080, .framerate = 30, .reg_val = 0x12 },    // 1080p30
	// 3G-SDI Double LVDS channels
	{.width = 1920, .height = 1080, .framerate = 50, .reg_val = 0x93 },    // 1080p50
	{.width = 1920, .height = 1080, .framerate = 60, .reg_val = 0x92 },    // 1080p60
};

#define SERIAL_BAUDRATE 9600
struct crosslink_ioctl_serial {
	u32 tx_len;
	u32 rx_len;
	u32 timeout_ms;
	u32 rx_wait_count;
	u8 tx_data[32];
	u8 rx_data[32];
};

struct crosslink_dev {
	struct device *dev;
	struct regmap *regmap;
	struct i2c_client *i2c_client;
	struct v4l2_subdev sd;
	struct media_pad pad;
	struct v4l2_fwnode_endpoint ep; /* the parsed DT endpoint info */
	struct gpio_desc *reset_gpio;
	struct mutex lock;
	struct v4l2_mbus_framefmt fmt;
	struct resolution *mode;
	char of_name[32];
	int framerate;
	int has_serial;
	int firmware_loaded;
};

enum crosslink_ioctl_cmds {
	CROSSLINK_CMD_GET_FW_REV  = 0x7600,
	CROSSLINK_CMD_SERIAL_TX   = 0x7601,
	CROSSLINK_CMD_SERIAL_RX   = 0x7602,
	CROSSLINK_CMD_SERIAL_XFER = 0x7603,
};

enum crosslink_regs {
	CROSSLINK_REG_ID = 0x1,         // RO: 8:  Firmware version
	CROSSLINK_REG_ENABLE = 0x2,     // RW: 8:  bit[0]: mipi-en, bit[1]: lvds-en, bit[2]: uart-en
	CROSSLINK_REG_MODE = 0x3,		// RW: 8:  mode, "reg_val" in struct resolution
	CROSSLINK_REG_LINE_COUNT = 0x4, // RO:16:  Y-count
	CROSSLINK_REG_COLM_COUNT = 0x6, // RO:16:  X-count
	CROSSLINK_REG_STATUS = 0x8, 	// RO: 8:  000 & pll_lock & gddr_rdy & bit_lock & word_lock & bw_rdy
	CROSSLINK_REG_UART_STAT = 0x9,  // RO: 8:  board_detect2 & board_detect1 & s_busy_tx & s_busy_rx & s_data_fulltx & s_data_emptytx & s_data_fullrx & s_data_emptyrx;
	CROSSLINK_REG_UART_RX_CNT= 0xA, // RO: 8:  count of bytes in UART RX fifo
	CROSSLINK_REG_UART_PRESCL= 0xB, // RW:16:  UART Prescaler from 24 MHz. default=2500 => 24M/2500=9600 baud.
	CROSSLINK_REG_SERIAL = 0x80,	// RW:XX:  Any bytes read/written above 0x80 are read from or written to the UART RX/TX fifos. Fifos are 32 bytes deep.
};

static inline struct crosslink_dev *to_crosslink_dev(struct v4l2_subdev *sd)
{
	return container_of(sd, struct crosslink_dev, sd);
}

static int crosslink_resolution_upcall(struct crosslink_dev *sensor, int resolution)
{
	char *argv[5], *envp[3];
	static char buf[32];

	dev_dbg_ratelimited(sensor->dev, "%s: \n", __func__);

	snprintf(buf, sizeof(buf), "0x%02x", resolution);

	argv[0] = "/bin/bash";
	argv[1] = "/bin/crosslink_cs1_res.sh";
	argv[2] = buf;
	argv[3] = NULL;

	/* minimal command environment taken from cpu_run_sbin_hotplug */
	envp[0] = "HOME=/home/root/";
	envp[1] = "PATH=/sbin:/bin:/usr/sbin:/usr/bin";
	envp[2] = NULL;

	call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
	return 0;
}

static void crosslink_power(struct crosslink_dev *sensor, int enable)
{
	dev_dbg(sensor->dev, "setting reset pin: %s\n", enable ? "ON" : "OFF");

	gpiod_set_value_cansleep(sensor->reset_gpio, enable ? 0 : 1);
	if (enable)
		msleep(150);
}

/* --------------- Subdev Operations --------------- */

static int crosslink_s_power(struct v4l2_subdev *sd, int on)
{
	struct crosslink_dev *sensor = to_crosslink_dev(sd);
	int ret;

	mutex_lock(&sensor->lock);
	ret = regmap_write(sensor->regmap, CROSSLINK_REG_ENABLE, on ? 0xFE : 0x00);
	msleep(5);
	// crosslink_power(sensor, on);
	mutex_unlock(&sensor->lock);
	return ret;
}

static int crosslink_xfer_serial(struct crosslink_dev *sensor, struct crosslink_ioctl_serial *serial)
{
	int ret=0;
	unsigned int rx_cnt;
	unsigned long expire;
	dev_dbg_ratelimited(sensor->dev, "%s: \n", __func__);
	print_hex_dump(KERN_DEBUG, "crosslink serial xfer: ", DUMP_PREFIX_NONE, 16, 1, serial->tx_data, 32, true);

	if (serial->tx_len != 0) {
		// write the data to the serial tx fifo
		ret |= regmap_bulk_write(sensor->regmap, CROSSLINK_REG_SERIAL, serial->tx_data, serial->tx_len);
		if (ret)
			return ret;
	}
	expire = jiffies + msecs_to_jiffies(serial->timeout_ms + 1);
	while (time_before(jiffies, expire)) {
		// wait for the rx fifo to fill up.
		ret |= regmap_read(sensor->regmap, CROSSLINK_REG_UART_RX_CNT, &rx_cnt);
		if (ret)
			return ret;
		if (serial->rx_wait_count) {
			if (rx_cnt >= serial->rx_wait_count)
			break;
		}
		usleep_range(1000, 3000);
	}
	print_hex_dump(KERN_DEBUG, "crosslink serial xfer: ", DUMP_PREFIX_NONE, 16, 1, serial->rx_data, 32, true);

	serial->rx_len = rx_cnt;
	ret |= regmap_bulk_read(sensor->regmap, CROSSLINK_REG_SERIAL, serial->rx_data, serial->rx_len);
	return ret;
}

static int crosslink_serial_tx_upcall(struct crosslink_dev *sensor, struct crosslink_ioctl_serial *serial)
{
	char *baud = "9600";
	char *tty = "/dev/ttymxc3";
	char cmd[512];
	char data[128];
	char *argv[] = { "/bin/sh", "-c", "X_cmd_X", NULL };
	char *envp[] = { "HOME=/", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };

	bin2hex(data, serial->tx_data, serial->tx_len);
	snprintf(cmd, sizeof(cmd), "/usr/bin/serial-xfer %s %s %s %d %d > /tmp/%s_out", baud, tty, data, serial->timeout_ms, serial->rx_wait_count, sensor->of_name);
	argv[2] = cmd;

	dev_dbg_ratelimited(sensor->dev, "%s: \n", __func__);

	return call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
}

static int crosslink_serial_rx_read(struct crosslink_dev *sensor, struct crosslink_ioctl_serial *serial)
{
	char path[128];
	char data[128];
	struct file *filp = NULL;
	ssize_t nbytes = 0;
	int retval = 0;

	snprintf(path, sizeof(path), "/tmp/%s_out", sensor->of_name);

	filp = filp_open(path, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		pr_err("%s unable to open %s file!\n", __func__, path);
		retval = PTR_ERR(filp);
	}
	else {
		nbytes = kernel_read(filp, &data, sizeof(data), 0);
		if (nbytes < 0) {
			pr_err("%s read %s file fail nbytes=%ld!\n", __func__, path, (long)nbytes);
			retval = -EIO;
		}
		else {
			hex2bin(serial->rx_data, data, nbytes);
			serial->rx_len = nbytes>>1;
			retval = 0;
		}
		filp_close(filp, NULL);
	}
	return retval;
}

static int crosslink_tty_xfer(struct crosslink_dev *sensor, struct crosslink_ioctl_serial *serial)
{
	int ret;

	dev_dbg_ratelimited(sensor->dev, "%s: \n", __func__);
	print_hex_dump(KERN_DEBUG, "crosslink tty xferTX: ", DUMP_PREFIX_NONE, 16, 1, serial->tx_data, 32, true);
	ret = crosslink_serial_tx_upcall(sensor, serial);
	dev_dbg_ratelimited(sensor->dev, "%s: %d\n", __func__, ret);
	if (ret < 0)
		return ret;

	ret = crosslink_serial_rx_read(sensor, serial);
	print_hex_dump(KERN_DEBUG, "crosslink tty xferRX: ", DUMP_PREFIX_NONE, 16, 1, serial->rx_data, 32, true);

	return ret;
}

static int crosslink_xfer(struct crosslink_dev *sensor, struct crosslink_ioctl_serial *serial)
{
	if (sensor->has_serial)
		return crosslink_xfer_serial(sensor, serial);
	else
		return crosslink_tty_xfer(sensor, serial);
}

static long crosslink_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	long ret = 0;
	unsigned int fw_rev;
	struct crosslink_dev *sensor = to_crosslink_dev(sd);
	struct crosslink_ioctl_serial *serial;

	mutex_lock(&sensor->lock);
	switch (cmd) {
		case CROSSLINK_CMD_GET_FW_REV:
			dev_dbg_ratelimited(sensor->dev, "%s: CROSSLINK_CMD_GET_FW_REV\n", __func__);
			ret = regmap_read(sensor->regmap, CROSSLINK_REG_ID, &fw_rev);
			*(u32*)arg = fw_rev;
			break;
		case CROSSLINK_CMD_SERIAL_TX:
			dev_dbg_ratelimited(sensor->dev, "%s: CROSSLINK_CMD_SERIAL_TX\n", __func__);
			serial = (struct crosslink_ioctl_serial *)arg;
			ret = regmap_bulk_write(sensor->regmap, CROSSLINK_REG_SERIAL, serial->tx_data, serial->tx_len);
			break;
		case CROSSLINK_CMD_SERIAL_RX:
			dev_dbg_ratelimited(sensor->dev, "%s: CROSSLINK_CMD_SERIAL_RX\n", __func__);
			serial = (struct crosslink_ioctl_serial *)arg;
			if (serial->rx_len == 0)
				ret = regmap_read(sensor->regmap, CROSSLINK_REG_UART_RX_CNT, &serial->rx_len);
			ret = regmap_bulk_read(sensor->regmap, CROSSLINK_REG_SERIAL, serial->rx_data, serial->rx_len);
			break;
		case CROSSLINK_CMD_SERIAL_XFER:
			dev_dbg_ratelimited(sensor->dev, "%s: CROSSLINK_CMD_SERIAL_XFER\n", __func__);
			serial = (struct crosslink_ioctl_serial *)arg;
			ret = crosslink_xfer(sensor, serial);
			break;
		default:
			ret = -EINVAL;
	}

	mutex_unlock(&sensor->lock);

	return 0;
}

static int ops_get_fmt(struct v4l2_subdev *sub_dev, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_format *format)
{
	struct crosslink_dev *sensor = to_crosslink_dev(sub_dev);
	struct v4l2_mbus_framefmt *fmt;

	dev_dbg(sub_dev->dev, "%s: \n", __func__);

	if (format->pad >= NUM_PADS)
		return -EINVAL;

	if (format->which == V4L2_SUBDEV_FORMAT_TRY)
		fmt = v4l2_subdev_get_try_format(&sensor->sd, sd_state, format->pad);
	else
		fmt = &sensor->fmt;

	format->format = *fmt;
	return 0;
}

static int crosslink_set_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_format *format)
{
	struct crosslink_dev *sensor = to_crosslink_dev(sd);
	const struct resolution *new_mode = NULL;
	struct v4l2_mbus_framefmt *fmt;
	int ret=0, i;

	dev_dbg(sd->dev, "%s: \n", __func__);

	if (format->pad >= NUM_PADS)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(sensor_res_list); i++) {
		if (format->format.width == sensor_res_list[i].width && format->format.height == sensor_res_list[i].height) {
			new_mode = &sensor_res_list[i];		// set mode here so illegal framerate just gets assigned a legal one.
			// a previous call to ops_set_frame_interval would have set sensor->framerate. Otherwise its the existing FR.
			if (sensor->framerate == sensor_res_list[i].framerate)
				break;
		}
	}
	if (new_mode == NULL)
		return -EINVAL;

	sensor->framerate = new_mode->framerate;

	if (format->which == V4L2_SUBDEV_FORMAT_TRY)
		fmt = v4l2_subdev_get_try_format(sd, sd_state, 0);
	else
		fmt = &sensor->fmt;

	*fmt = format->format;

	// if ((new_mode != sensor->mode) && (format->which != V4L2_SUBDEV_FORMAT_TRY)) {
	sensor->mode = new_mode;
	ret |= crosslink_resolution_upcall(sensor, new_mode->reg_val);

	pr_debug("%s: sensor->ep.bus_type =%d\n", __func__, sensor->ep.bus_type);
	pr_debug("%s: sensor->ep.bus      =%p\n", __func__, &sensor->ep.bus);
	pr_debug("%s: sensor->fmt->height =%d\n", __func__, format->format.height);
	pr_debug("%s: sensor->fmt->width  =%d\n", __func__, format->format.width);
	pr_debug("%s: end \n", __func__);
	return ret;
}

static int ops_enum_frame_size(struct v4l2_subdev *sub_dev, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_frame_size_enum *fse)
{
	struct crosslink_dev *sensor = to_crosslink_dev(sub_dev);
	struct resolution res = {.width=U16_MAX, .height=U16_MAX};
	int i, unique=0;

	if (fse->pad >= NUM_PADS)
		return -EINVAL;
	if (fse->code != sensor->fmt.code) {
		dev_dbg_ratelimited(sub_dev->dev, "%s unsupported fmt.code: 0x%04x.\n", __func__, fse->code);
		return -EINVAL;
	}

	// return the unique resoluitons.
	for(i = 0 ; i < ARRAY_SIZE(sensor_res_list) ; i++) {
		if(res.width != sensor_res_list[i].width || res.height != sensor_res_list[i].height) {
			res = sensor_res_list[i];
			if (fse->index == unique++) {
				fse->min_width  = fse->max_width  = res.width;
				fse->min_height = fse->max_height = res.height;
				dev_dbg_ratelimited(sub_dev->dev, "%s: offer size WxH@mode=%dx%d for mode 0x%04x.\n", __func__, res.width, res.height, fse->code);
				return 0;
			}
		}
	}
	return -EINVAL;
}

static int ops_enum_frame_interval(struct v4l2_subdev *sub_dev, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_frame_interval_enum *fie)
{
	struct crosslink_dev *sensor = to_crosslink_dev(sub_dev);
	int i, count=0;

	if (fie->pad >= NUM_PADS)
		return -EINVAL;
	// only suports MEDIA_BUS_FMT_YUYV8_1X16
	if (fie->code != sensor->fmt.code) {
		dev_dbg_ratelimited(sub_dev->dev, "%s unsupported fmt.code: 0x%04x.\n", __func__, fie->code);
		return -EINVAL;
	}

	if (fie->width == 0 || fie->height == 0 || fie->code == 0) {
		dev_err_ratelimited(sub_dev->dev, "%s Please assign pix-fmt & resolution before enum framerates.\n", __func__);
		return -EINVAL;
	}

	fie->interval.numerator = 1;

	// find the n-th entry that matches the given resolution.
	for(i=0; i < ARRAY_SIZE(sensor_res_list); i++) {
		if(fie->width == sensor_res_list[i].width && fie->height == sensor_res_list[i].height) {
			if (fie->index == count++) {
				fie->interval.denominator = sensor_res_list[i].framerate;
				dev_dbg_ratelimited(sub_dev->dev, "%s: offer framerate %dfps for WxH@mode=%dx%d@0x%04x.\n", __func__, fie->interval.denominator, fie->width, fie->height, fie->code);
				return 0;
			}
		}
	}
	return -EINVAL;
}

static int ops_get_frame_interval(struct v4l2_subdev *sub_dev, struct v4l2_subdev_frame_interval *fi)
{
	struct crosslink_dev *sensor = to_crosslink_dev(sub_dev);

	dev_dbg(sub_dev->dev, "%s: \n", __func__);

	if (fi->pad >= NUM_PADS)
		return -EINVAL;

	fi->interval.numerator = 1;
	fi->interval.denominator = sensor->mode->framerate;

	return 0;
}

static int ops_set_frame_interval(struct v4l2_subdev *sub_dev, struct v4l2_subdev_frame_interval *fi)
{
	int i;
	struct crosslink_dev *sensor = to_crosslink_dev(sub_dev);

	dev_dbg(sub_dev->dev, "%s(setting interval = %d)\n", __func__, fi->interval.denominator);
	for(i=0; i < ARRAY_SIZE(sensor_res_list); i++) {
		if(sensor_res_list[i].framerate == fi->interval.denominator) {
			sensor->framerate = fi->interval.denominator;
			return 0;
		}
	}

	dev_err(sensor->dev, "unsupported framerate: %d\n", fi->interval.denominator);
	return -EINVAL;
}

static int crosslink_enum_mbus_code(struct v4l2_subdev *sub_dev, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_mbus_code_enum *code)
{
	dev_dbg_ratelimited(sub_dev->dev, "%s: \n", __func__);

	if ((code->pad >= NUM_PADS) || (code->index >= 1))
		return -EINVAL;

	code->code = MEDIA_BUS_FMT_YUYV8_1X16;
	return 0;
}

static int crosslink_soft_reset(struct crosslink_dev *sensor, bool enable)
{
	dev_dbg_ratelimited(sensor->dev, "%s: \n", __func__);
	return regmap_write(sensor->regmap, 0x2, enable ? 0xFF : 0xFE);
}

static int crosslink_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct crosslink_dev *sensor = to_crosslink_dev(sd);
	int ret = 0;

	if (sensor->ep.bus_type != V4L2_MBUS_CSI2_DPHY){
		dev_err(sensor->dev, "endpoint bus_type not supported: %d\n", sensor->ep.bus_type);
		return -EINVAL;
	}

	mutex_lock(&sensor->lock);
	ret = regmap_write(sensor->regmap, 0x3, sensor->mode->reg_val);
	ret |= crosslink_soft_reset(sensor, 0);
	if (enable)
		msleep(50);
	ret |= crosslink_soft_reset(sensor, enable);
	mutex_unlock(&sensor->lock);
	if (enable)
		pr_debug("%s: Starting stream at WxH@fps=%dx%d@%d\n", __func__, sensor->mode->width, sensor->mode->height, sensor->mode->framerate);
	else
		pr_debug("%s: Stopping stream \n", __func__);

	return ret;
}

extern int crosslink_fpga_ops_write_init(struct gpio_desc *reset, struct i2c_client *client);
extern int crosslink_fpga_ops_write(struct i2c_client *client, const char *buf, size_t count);
extern int crosslink_fpga_ops_write_complete(struct i2c_client *client);
static void crosslink_remove(struct i2c_client *client);

static void crosslink_fw_handler(const struct firmware *fw, void *context)
{
	int ret;
	struct crosslink_dev *sensor = (struct crosslink_dev *)context;

	if (!fw)
		return;

	mutex_lock(&sensor->lock);
	ret = crosslink_fpga_ops_write_init(sensor->reset_gpio, sensor->i2c_client);
	if (ret < 0)
		goto exit;
	ret = crosslink_fpga_ops_write(sensor->i2c_client, fw->data, fw->size);
	if (ret < 0)
		goto exit;
	ret = crosslink_fpga_ops_write_complete(sensor->i2c_client);
	if (ret < 0)
		goto exit;

	sensor->firmware_loaded = 1;
exit:
	release_firmware(fw);
	mutex_unlock(&sensor->lock);
	if (ret < 0) {
		dev_err(sensor->dev, "Failed to load firmware: %d\n", ret);
		crosslink_remove(sensor->i2c_client);
	}
}


static const struct v4l2_subdev_core_ops crosslink_core_ops = {
	.s_power = crosslink_s_power,
	.log_status = v4l2_ctrl_subdev_log_status,
	.subscribe_event = v4l2_ctrl_subdev_subscribe_event,
	.unsubscribe_event = v4l2_event_subdev_unsubscribe,
	.ioctl = crosslink_ioctl,
};

static const struct v4l2_subdev_video_ops crosslink_video_ops = {
	.g_frame_interval = ops_get_frame_interval,
	.s_frame_interval = ops_set_frame_interval,
	.s_stream = crosslink_s_stream,
};

static const struct v4l2_subdev_pad_ops crosslink_pad_ops = {
	.enum_mbus_code = crosslink_enum_mbus_code,
	.get_fmt = ops_get_fmt,
	.set_fmt = crosslink_set_fmt,
	.enum_frame_size = ops_enum_frame_size,
	.enum_frame_interval = ops_enum_frame_interval,
};

static const struct v4l2_subdev_ops crosslink_subdev_ops = {
	.core = &crosslink_core_ops,
	.video = &crosslink_video_ops,
	.pad = &crosslink_pad_ops,
};

static int crosslink_link_setup(struct media_entity *entity, const struct media_pad *local, const struct media_pad *remote, u32 flags)
{
	return 0;
}

static const struct media_entity_operations crosslink_sd_media_ops = {
	.link_setup = crosslink_link_setup,
};

static const struct regmap_config sensor_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.cache_type = REGCACHE_NONE,
};

static int crosslink_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct fwnode_handle *endpoint;
	struct crosslink_dev *sensor;
	struct v4l2_mbus_framefmt *fmt;
	int ret;
	unsigned int id_code=0;
	unsigned int uart_stat;

	pr_debug("-->%s crosslink Probe start\n",__func__);

	sensor = devm_kzalloc(dev, sizeof(*sensor), GFP_KERNEL);
	if (!sensor)
		return -ENOMEM;

	sensor->dev = dev;
	sensor->i2c_client = client;

	// default init sequence initialize sensor to 1080p30 YUV422 UYVY
	fmt = &sensor->fmt;
	fmt->code = MEDIA_BUS_FMT_YUYV8_1X16;
	fmt->colorspace = V4L2_COLORSPACE_SRGB;
	fmt->ycbcr_enc = V4L2_MAP_YCBCR_ENC_DEFAULT(fmt->colorspace);
	fmt->quantization = V4L2_QUANTIZATION_FULL_RANGE;
	fmt->xfer_func = V4L2_MAP_XFER_FUNC_DEFAULT(fmt->colorspace);
	fmt->width = 1280;
	fmt->height = 720;
	fmt->field = V4L2_FIELD_NONE;
	sensor->framerate = 30;
	sensor->mode = &sensor_res_list[0];

	/* request reset pin */
	sensor->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(sensor->reset_gpio)) {
		ret = PTR_ERR(sensor->reset_gpio);
		if (ret != -EPROBE_DEFER)
			dev_warn(dev, "Cannot get reset GPIO (%d)", ret);
		// return ret;
	}

	// get name
	snprintf(sensor->of_name, 31, "VD_%s_%d", client->name, client->addr);
	sensor->of_name[31] = 0;

	endpoint = fwnode_graph_get_next_endpoint(dev_fwnode(&client->dev), NULL);
	if (!endpoint) {
		dev_err(dev, "endpoint node not found\n");
		return -EINVAL;
	}

	ret = v4l2_fwnode_endpoint_parse(endpoint, &sensor->ep);
	fwnode_handle_put(endpoint);
	if (ret) {
		dev_err(dev, "Could not parse endpoint\n");
		return ret;
	}

	pr_debug("%s: sensor->ep.bus_type=%d\n", __func__, sensor->ep.bus_type);

	if (sensor->ep.bus_type != V4L2_MBUS_CSI2_DPHY) {
		dev_err(dev, "Unsupported bus type %d\n", sensor->ep.bus_type);
		return -EINVAL;
	}

	sensor->regmap = devm_regmap_init_i2c(client, &sensor_regmap_config);
	if (IS_ERR(sensor->regmap)) {
		dev_err(dev, "regmap init failed\n");
		return PTR_ERR(sensor->regmap);
	}

	ret = regmap_read(sensor->regmap, CROSSLINK_REG_ID, &id_code);
	if (ret)
		dev_dbg(dev, "Could not read device-id. trying again\n");
	// if (id_code != FIRMWARE_VERSION) {
		dev_info(dev, "Loading current Firmware: %02x\n", FIRMWARE_VERSION);
		ret = request_firmware_nowait(THIS_MODULE, FW_ACTION_UEVENT, FIRWARE_NAME, dev, GFP_KERNEL, sensor, crosslink_fw_handler);
		if (ret) {
			dev_err(dev, "Failed request_firmware_nowait err %d\n", ret);
			goto entity_cleanup;
		}
	// }

	v4l2_i2c_subdev_init(&sensor->sd, client, &crosslink_subdev_ops);

	sensor->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS | V4L2_SUBDEV_FL_HAS_DEVNODE;
	sensor->sd.dev = &client->dev;
	sensor->pad.flags = MEDIA_PAD_FL_SOURCE;
	sensor->sd.entity.ops = &crosslink_sd_media_ops;
	sensor->sd.entity.function = MEDIA_ENT_F_CAM_SENSOR;
	ret = media_entity_pads_init(&sensor->sd.entity, 1, &sensor->pad);
	if (ret)
		return ret;

	mutex_init(&sensor->lock);

	// get the uart status reg to see if the crosslink board rev supports i2c->serial passthrough.
	ret = regmap_read(sensor->regmap, CROSSLINK_REG_UART_STAT, &uart_stat);
	if (ret == 0)
		sensor->has_serial = uart_stat & 0b11000000;

	// turn off
	crosslink_power(sensor, 0);

	ret = v4l2_async_register_subdev_sensor(&sensor->sd);
	if (ret)
		goto entity_cleanup;

	pr_debug("<--%s crosslink Probe end successful, return\n",__func__);
	return 0;

entity_cleanup:
	pr_debug("---%s crosslink ERR entity_cleanup\n",__func__);
	media_entity_cleanup(&sensor->sd.entity);
	mutex_destroy(&sensor->lock);
	return ret;
}

static void crosslink_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct crosslink_dev *sensor = to_crosslink_dev(sd);

	v4l2_async_unregister_subdev(&sensor->sd);
	media_entity_cleanup(&sensor->sd.entity);
	mutex_destroy(&sensor->lock);
}

static const struct i2c_device_id crosslink_id[] = {
	{"crosslink", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, crosslink_id);

static const struct of_device_id crosslink_dt_ids[] = {
	{ .compatible = "scailx,crosslink" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, crosslink_dt_ids);

static struct i2c_driver crosslink_i2c_driver = {
	.driver = {
		.name  = "crosslink",
		.of_match_table	= crosslink_dt_ids,
	},
	.id_table = crosslink_id,
	.probe_new = crosslink_probe,
	.remove   = crosslink_remove,
};

module_i2c_driver(crosslink_i2c_driver);

MODULE_DESCRIPTION("crosslink MIPI Camera Subdev Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("VideologyInc, 2023");
MODULE_VERSION("1.0");
MODULE_ALIAS("crosslink_lvds2mipi");