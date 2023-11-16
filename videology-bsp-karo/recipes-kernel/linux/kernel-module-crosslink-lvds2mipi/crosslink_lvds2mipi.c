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
#include <linux/regmap.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
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

struct crosslink_dev {
	struct device *dev;
	struct regmap *regmap;
	struct v4l2_subdev sd;
	struct media_pad pad;
	struct v4l2_fwnode_endpoint ep; /* the parsed DT endpoint info */
	struct gpio_desc *reset_gpio;
	struct mutex lock;
	struct v4l2_mbus_framefmt fmt;
	const struct resolution *mode;
	int framerate;
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
	mutex_lock(&sensor->lock);
	// crosslink_power(sensor, on);
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
	return regmap_write(sensor->regmap, 0x2, enable ? 0xff : 0);
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

static const struct v4l2_subdev_core_ops crosslink_core_ops = {
	.s_power = crosslink_s_power,
	.log_status = v4l2_ctrl_subdev_log_status,
	.subscribe_event = v4l2_ctrl_subdev_subscribe_event,
	.unsubscribe_event = v4l2_event_subdev_unsubscribe,
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
	unsigned int id_code;

	pr_debug("-->%s crosslink Probe start\n",__func__);

	sensor = devm_kzalloc(dev, sizeof(*sensor), GFP_KERNEL);
	if (!sensor)
		return -ENOMEM;

	sensor->dev = dev;

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
			dev_err(dev, "Cannot get reset GPIO (%d)", ret);
		return ret;
	}

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

	ret = regmap_read(sensor->regmap, 0x1, &id_code);
	if (ret) {
		/* If we can't read the ID, it may be that the FPGA hasn't loaded yet after releasing reset. */
		dev_dbg(dev, "Could not read device-id. trying again\n");
		return -EPROBE_DEFER;
	}
	else {
		dev_info(dev, "Got device-id %02x\n", id_code);
	}

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

	//turn off
	// crosslink_power(sensor, 0);

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
