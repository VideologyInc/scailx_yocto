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


#define V4L2_CID_ZOOM_SPEED		(V4L2_CID_CAMERA_CLASS_BASE+50)
#define V4L2_CID_NOISE_RED      (V4L2_CID_BASE+50)


struct resolution {
	u16 width;
	u16 height;
	u16 framerate;
	u16 frame_format_code;
	char *name;
};

static struct resolution sensor_res_list[] = {
	{.width = 1280, .height = 720,  .framerate = 25, .frame_format_code = 12, .name="720p25"  },
	{.width = 1280, .height = 720,  .framerate = 30, .frame_format_code = 12, .name="720p30"  },
	{.width = 1280, .height = 720,  .framerate = 50, .frame_format_code = 12, .name="720p50"  },
	{.width = 1280, .height = 720,  .framerate = 60, .frame_format_code = 12, .name="720p60"  },

	{.width = 1280, .height = 960,  .framerate = 25, .frame_format_code = 9, .name="960p25"  },
	{.width = 1280, .height = 960,  .framerate = 30, .frame_format_code = 9, .name="960p30"  },
	{.width = 1280, .height = 960,  .framerate = 50, .frame_format_code = 9, .name="960p50"  },
	{.width = 1280, .height = 960,  .framerate = 60, .frame_format_code = 9, .name="960p60"  },

	{.width = 1920, .height = 1080, .framerate = 25, .frame_format_code = 3, .name="1080p25" },
	{.width = 1920, .height = 1080, .framerate = 30, .frame_format_code = 3, .name="1080p30" },
	{.width = 1920, .height = 1080, .framerate = 50, .frame_format_code = 3, .name="1080p50" },
	{.width = 1920, .height = 1080, .framerate = 60, .frame_format_code = 3, .name="1080p60" },

	{.width = 1440, .height = 1080, .framerate = 25, .frame_format_code = 4, .name="1080x1080@25" },
	{.width = 1440, .height = 1080, .framerate = 30, .frame_format_code = 4, .name="1080x1080@30" },
	{.width = 1440, .height = 1080, .framerate = 50, .frame_format_code = 4, .name="1080x1080@50" },
	{.width = 1440, .height = 1080, .framerate = 60, .frame_format_code = 4, .name="1080x1080@60" },

	{.width = 1080, .height = 1080, .framerate = 25, .frame_format_code = 5, .name="1080x1080@25" },
	{.width = 1080, .height = 1080, .framerate = 30, .frame_format_code = 5, .name="1080x1080@30" },
	{.width = 1080, .height = 1080, .framerate = 50, .frame_format_code = 5, .name="1080x1080@50" },
	{.width = 1080, .height = 1080, .framerate = 60, .frame_format_code = 5, .name="1080x1080@60" },

	{.width = 1024, .height = 1024, .framerate = 25, .frame_format_code = 11, .name="1024x1024@25" },
	{.width = 1024, .height = 1024, .framerate = 30, .frame_format_code = 11, .name="1024x1024@30" },
	{.width = 1024, .height = 1024, .framerate = 50, .frame_format_code = 11, .name="1024x1024@50" },
	{.width = 1024, .height = 1024, .framerate = 60, .frame_format_code = 11, .name="1024x1024@60" },

	{.width = 1280, .height = 1024, .framerate = 25, .frame_format_code = 7, .name="1280x1024@25" },
	{.width = 1280, .height = 1024, .framerate = 30, .frame_format_code = 7, .name="1280x1024@30" },
	{.width = 1280, .height = 1024, .framerate = 50, .frame_format_code = 7, .name="1280x1024@50" },
	{.width = 1280, .height = 1024, .framerate = 60, .frame_format_code = 7, .name="1280x1024@60" },
};

enum commands {
	GS_COMD_8BIT_REG_W =            0x30,
	GS_COMD_8BIT_REG_R =            0x31,
	GS_COMD_16BIT_REG_W =           0x32,
	GS_COMD_16BIT_REG_R =           0x33,
	GS_COMD_32BIT_REG_W =           0x34,
	GS_COMD_32BIT_REG_R =           0x35,
	GS_COMD_ISP_FLASH_W =           0x40,
	GS_COMD_ISP_FLASH_R =           0x41,
	GS_COMD_ISP_FLASH_ERASE =       0x42,
	// Password protected commands
	GS_COMD_ISP_FLASH_GET_ID =      0x43,
	GS_COMD_ISP_FLASH_BLOCK_ERASE = 0x44,
	GS_COMD_ISP_FLASH_GET_STAT =    0x45,
	GS_COMD_ISP_FLASH_GET_CRC =     0x47,
	GS_COMD_NVM_W =                 0x50,
	GS_COMD_NVM_R =                 0x51,
	GS_COMD_NVM_ERASE =             0x52,
	GS_COMD_R_SERIAL =              0x61,
};

enum regs {
	GS_REG_BRIGHTNESS     	= 0x02,
	GS_REG_CONTRAST       	= 0x04,
	GS_REG_SATURATION     	= 0x06,
	GS_REG_SHARPNESS      	= 0x0A,
	GS_REG_NOISE_RED      	= 0x0C,
	GS_REG_GAMMA		  	= 0x0E,
	GS_REG_ZOOM				= 0x18,
	GS_REG_ZOOM_SPEED		= 0x1C,
	GS_REG_PAN				= 0x1D,
	GS_REG_TILT				= 0x1E,
	GS_REG_MIRROR_FLIP    	= 0x1F,
	GS_REG_EXPOSURE_MODE  	= 0x20,
	GS_REG_GAIN				= 0x30,
	GS_REG_EXPOSURE_ABS    	= 0x34,
	GS_REG_AE_TARGET      	= 0x3C,
	GS_REG_BLC_MODE			= 0x40,
	GS_REG_BLC_LEVEL		= 0x41,
	GS_REG_WHITEBALANCE   	= 0x50,
	GS_REG_WB_TEMPERATURE 	= 0x52,
	GS_REG_ANTIFLICKER_MODE = 0x56,
	GS_REG_ANTIFLICKER_FREQ = 0x57,
	GS_REG_COLORFX			= 0x76,
	GS_REG_TESTPATTERN 		= 0xE0,
};

enum colorformat {
	GS_CF_YUV422 = 0,
	GS_CF_YUV420,
	GS_CF_YUV400,
	GS_CF_YUV422BT,
	GS_CF_YUV420BT,
	GS_CF_YUV400BT,
	GS_CF_RGB_888,
	GS_CF_RGB_565,
	GS_CF_RGB_555,
	GS_CF_JPEG422,
	GS_CF_JPEG420,
	GS_CF_JPEG422EX,
	GS_CF_JPEG420EX,
	GS_CF_BAYER_16,
	GS_CF_BAYER_12,
	GS_CF_BAYER_10,
	GS_CF_BAYER_8,
};


struct gs_ar0234_ctrls {
	struct v4l2_ctrl_handler handler;
	// struct v4l2_ctrl *pixel_rate;
	struct v4l2_ctrl *auto_exp;
	struct v4l2_ctrl *exposure;
	struct v4l2_ctrl *exposure_absolute;
	struct v4l2_ctrl *exposure_metering; // =blc mode
	struct v4l2_ctrl *blc_level;
	struct v4l2_ctrl *auto_wb;
	struct v4l2_ctrl *push_to_white;
	struct v4l2_ctrl *wb_temp;
	struct v4l2_ctrl *wb_preset;
	// struct v4l2_ctrl *blue_balance;
	// struct v4l2_ctrl *red_balance;
	// struct v4l2_ctrl *auto_gain;
	struct v4l2_ctrl *gain;
	struct v4l2_ctrl *brightness;
	// struct v4l2_ctrl *light_freq;
	struct v4l2_ctrl *saturation;
	struct v4l2_ctrl *contrast;
	struct v4l2_ctrl *sharpness;
	struct v4l2_ctrl *noise_red;
	struct v4l2_ctrl *gamma;
	// struct v4l2_ctrl *hue;
	struct v4l2_ctrl *hflip;
	struct v4l2_ctrl *vflip;
	struct v4l2_ctrl *powerline;
	struct v4l2_ctrl *testpattern;
	struct v4l2_ctrl *colorfx;
	struct v4l2_ctrl *zoom;
	struct v4l2_ctrl *zoom_speed;
	struct v4l2_ctrl *pan;
	struct v4l2_ctrl *tilt;
};

struct gs_ar0234_dev {
	struct device *dev;
	struct regmap *regmap;
	struct i2c_client *i2c_client;
	struct v4l2_subdev sd;
	struct media_pad pad;
	struct v4l2_fwnode_endpoint ep; /* the parsed DT endpoint info */
	struct gpio_desc *reset_gpio;
	struct mutex lock;
	struct v4l2_mbus_framefmt fmt;
	struct gs_ar0234_ctrls ctrls;
	const struct resolution *mode;
	int mbus_num;
	int framerate;
};

static inline struct gs_ar0234_dev *to_gs_ar0234_dev(struct v4l2_subdev *sd)
{
	return container_of(sd, struct gs_ar0234_dev, sd);
}

static inline struct v4l2_subdev *ctrl_to_sd(struct v4l2_ctrl *ctrl)
{
	return &container_of(ctrl->handler, struct gs_ar0234_dev,
			     ctrls.handler)->sd;
}

#define I2C_RETRIES 50
static int gs_ar0234_i2c_trx_retry(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	int i, ret;
	for (i = 0; i < I2C_RETRIES; i++) {
		ret = i2c_transfer(adap, msgs, num);
		if (ret >= 0)
			break;
		msleep(1);
	}
	return ret;
	// return 0;
}

static int gs_ar0234_read_reg8(struct gs_ar0234_dev *sensor, u8 addr, u8 *val)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg[2];
	u8 buf[2];
	int ret;

	buf[0] = 0x31;
	buf[1] = addr;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = buf;
	msg[0].len = sizeof(buf);

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = 1;
	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	*val = buf[0];
	return 0;
}

static int gs_ar0234_read_reg16(struct gs_ar0234_dev *sensor, u8 addr, u16 *val)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg[2];
	u8 buf[2];
	int ret;

	buf[0] = 0x33;
	buf[1] = addr;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = buf;
	msg[0].len = sizeof(buf);

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = 2;

	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	*val = ((u16)buf[1] << 8) | (u16)buf[0];
	return 0;
}

static int gs_ar0234_read_reg32(struct gs_ar0234_dev *sensor, u8 addr, u32 *val)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg[2];
	u8 buf[2];
	u8 bufo[4];
	int ret;

	buf[0] = 0x35;
	buf[1] = addr;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = buf;
	msg[0].len = sizeof(buf);

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].buf = bufo;
	msg[1].len = 4;

	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	*val = ((u32)bufo[3] << 24) | ((u32)bufo[2] << 16) | ((u32)bufo[1] << 8) | (u32)bufo[0];
	return 0;
}

static int gs_ar0234_write_reg8(struct gs_ar0234_dev *sensor, u8 addr, u8 val)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 buf[3];
	int ret;

	buf[0] = 0x30;
	buf[1] = addr;
	buf[2] = val;

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = buf;
	msg.len = sizeof(buf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	return 0;
}

static int gs_ar0234_write_reg16(struct gs_ar0234_dev *sensor, u8 addr, u16 val)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 buf[4];
	int ret;

	buf[0] = 0x32;
	buf[1] = addr;
	buf[2] = val & 0xff;
	buf[3] = (val >> 8) & 0xFF;

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = buf;
	msg.len = sizeof(buf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	return 0;
}

static int gs_ar0234_write_reg32(struct gs_ar0234_dev *sensor, u8 addr, u32 val)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 buf[6];
	int ret;

	buf[0] = 0x34;
	buf[1] = addr;
	buf[2] = val & 0xff;
	buf[3] = (val >> 8) & 0xff;
	buf[4] = (val >> 16) & 0xff;
	buf[5] = (val >> 24) & 0xff;

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = buf;
	msg.len = sizeof(buf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	return 0;
}

static void gs_ar0234_power(struct gs_ar0234_dev *sensor, int enable)
{
	dev_dbg(sensor->dev, "setting reset pin: %s\n", enable ? "ON" : "OFF");

	gpiod_set_value_cansleep(sensor->reset_gpio, enable ? 0 : 1);
	if (enable)
		msleep(150);
}

/* --------------- Subdev Operations --------------- */

static int gs_ar0234_s_power(struct v4l2_subdev *sd, int on)
{
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sd);
	mutex_lock(&sensor->lock);
	dev_info(sensor->dev, "%s: %s\n", __func__, on ? "ON" : "OFF");
	//////////////////////////////////gs_ar0234_power(sensor, on);
	mutex_unlock(&sensor->lock);
	return 0;
}

static int ops_get_fmt(struct v4l2_subdev *sub_dev, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_format *format)
{
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sub_dev);
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

static int gs_ar0234_set_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_format *format)
{
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sd);
	const struct resolution *new_mode = NULL;
	struct v4l2_mbus_framefmt *fmt = &format->format;
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



	// TODO : implement function to set media-bus format and colorspace based on fmt





	/////////////////////////////////////////////////////////////////////////////////


	sensor->framerate = new_mode->framerate;

	if (format->which == V4L2_SUBDEV_FORMAT_TRY)
		fmt = v4l2_subdev_get_try_format(sd, sd_state, 0);
	else
		fmt = &sensor->fmt;

	*fmt = format->format;

	// if ((new_mode != sensor->mode) && (format->which != V4L2_SUBDEV_FORMAT_TRY)) {
	sensor->mode = new_mode;

	pr_debug("%s: sensor->ep.bus_type =%d\n", __func__, sensor->ep.bus_type);
	pr_debug("%s: sensor->ep.bus      =%p\n", __func__, &sensor->ep.bus);
	pr_debug("%s: sensor->fmt->height =%d\n", __func__, format->format.height);
	pr_debug("%s: sensor->fmt->width  =%d\n", __func__, format->format.width);
	pr_debug("%s: end \n", __func__);
	return ret;
}

static int gs_ar0234_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sd);
	int ret;
	u16 shortval;

	/* v4l2_ctrl_lock() locks our own mutex */
	dev_dbg_ratelimited(sd->dev, "%s %x: \n", __func__,ctrl->id);

	switch (ctrl->id) {
		case V4L2_CID_BRIGHTNESS:
			ret = gs_ar0234_read_reg16(sensor, GS_REG_BRIGHTNESS, &shortval);
			if (ret < 0)
				return ret;
			sensor->ctrls.brightness->val = shortval;
			break;

		default:
			ret = -EINVAL;
	}

	return ret;
}

static int gs_ar0234_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sd);
	int ret, val;
	u8 val8;
	u16 tmp;

	// if (sensor->power_count == 0)
	// 	return 0;
	dev_dbg_ratelimited(sd->dev, "%s: \n", __func__);

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_BRIGHTNESS, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set brightness to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_CONTRAST:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_CONTRAST, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set contrast to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_SATURATION:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_SATURATION, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set saturation to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_AUTO_WHITE_BALANCE:
			ret = gs_ar0234_write_reg8(sensor, GS_REG_WHITEBALANCE, ctrl->val == 0 ? 0x7 : 0xF); // only OFF and ON
			dev_dbg_ratelimited(sd->dev, "%s: set white balance to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_DO_WHITE_BALANCE:
			ret = gs_ar0234_write_reg8(sensor, GS_REG_WHITEBALANCE, 0x8); // push to white, when done AWB is in manual mode
			dev_dbg_ratelimited(sd->dev, "%s: set push_to_white to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_WB_TEMPERATURE, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set white balance temperature to %d K\n", __func__, ctrl->val);
		break;
	case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
		switch(ctrl->val) {
			//case V4L2_WHITE_BALANCE_MANUAL: 		val = 0; break;
			//case V4L2_WHITE_BALANCE_AUTO: 		val = 0; break;
			case V4L2_WHITE_BALANCE_INCANDESCENT: 	val = 3000; break;
			case V4L2_WHITE_BALANCE_FLUORESCENT: 	val = 4000; break;
			case V4L2_WHITE_BALANCE_FLUORESCENT_H: 	val = 5000; break;
			case V4L2_WHITE_BALANCE_HORIZON: 		val = 5000; break;
			case V4L2_WHITE_BALANCE_DAYLIGHT: 		val = 6500; break;
			case V4L2_WHITE_BALANCE_FLASH: 			val = 5500; break;
			case V4L2_WHITE_BALANCE_CLOUDY: 		val = 7500; break;
			case V4L2_WHITE_BALANCE_SHADE: 			val = 9500; break;
		}
		ret = gs_ar0234_write_reg16(sensor, GS_REG_WB_TEMPERATURE, val);
		dev_dbg_ratelimited(sd->dev, "%s: set white balance temperature to %d K\n", __func__, val);
		break;
	case V4L2_CID_RED_BALANCE: // Red gain in manual WB
		break;
	case V4L2_CID_BLUE_BALANCE: // Blue gain in manual WB
		break;
	case V4L2_CID_GAMMA:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_GAMMA, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set gamma to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_EXPOSURE_AUTO: // exposure menu selection
		switch(ctrl->val) {
			case V4L2_EXPOSURE_AUTO:
				val8 = 0xC; // Auto Brightness/Exposure/Gain
				break;
			case V4L2_EXPOSURE_MANUAL:
				val8 = 0x0; // Manual Exposure, manual Gain
				break;
			case V4L2_EXPOSURE_SHUTTER_PRIORITY:
				val8 = 0x9; // Auto BV, Manual Exposure, Auto Gain
				break;
			case V4L2_EXPOSURE_APERTURE_PRIORITY:
				val8 = 0xC; //  For now use same as Auto
				break;
			default:
				val8 = 0xFF; // ignore for others
				break;
		}
		if(val8 < 0xFF) 	{
			ret = gs_ar0234_write_reg8(sensor, GS_REG_EXPOSURE_MODE, val8);
			dev_dbg_ratelimited(sd->dev, "%s: set exposure mode to %d\n", __func__, val8);
		}
		break;
	case V4L2_CID_EXPOSURE_ABSOLUTE : // reference level in (us * 100) in order to fit in 16bit:333 = 33300us
		val = ctrl->val * 100;
		ret = gs_ar0234_write_reg32(sensor, GS_REG_EXPOSURE_ABS, val);
		dev_dbg_ratelimited(sd->dev, "%s: set exposure to %d us\n", __func__, val);
		break;
	case V4L2_CID_EXPOSURE : // for now use to adjust reference or target level
		val = ctrl->val;
		//need to convert val to format s7.8
		tmp = (u16) (val * 256 / 1000);  // reverse = val * 1000 / 256
		tmp = val < 0 ? tmp - ((val%1000) > -500 ? 0 : 1) : tmp + ((val%1000) < 500 ? 0 : 1); // rounding
		ret = gs_ar0234_write_reg16(sensor, GS_REG_AE_TARGET, tmp);
		dev_dbg_ratelimited(sd->dev, "%s: set exposure target to 0x%04X\n", __func__, tmp);
		break;
	case V4L2_CID_GAIN: // gain level
		ret = gs_ar0234_write_reg16(sensor, GS_REG_GAIN, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set gain to %d\n", __func__, ctrl->val);
		break;

	case V4L2_CID_AUTOGAIN: //  ??????<tbd>
		break;
	case V4L2_CID_AUTOBRIGHTNESS: //  ?????? <tbd>
		break;
	case V4L2_CID_ISO_SENSITIVITY: //  ?????? <tbd>
		break;
	case V4L2_CID_ISO_SENSITIVITY_AUTO: //  ?????? <tbd>
		break;

	case V4L2_CID_EXPOSURE_METERING:
		switch(ctrl->val) {
			case V4L2_EXPOSURE_METERING_AVERAGE: 			val8 = 0; break;
			case V4L2_EXPOSURE_METERING_CENTER_WEIGHTED: 	val8 = 1; break;
			case V4L2_EXPOSURE_METERING_SPOT: 				val8 = 2; break;
			case V4L2_EXPOSURE_METERING_MATRIX: 			val8 = 3; break; // apointer to 8 weight table
		}
		ret = gs_ar0234_write_reg8(sensor, GS_REG_BLC_MODE, val8);
		dev_dbg_ratelimited(sd->dev, "%s: set exopure metering to %d\n", __func__, val8);
		break;
	case V4L2_CID_BACKLIGHT_COMPENSATION:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_BLC_LEVEL, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set blc level to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_POWER_LINE_FREQUENCY:
		ret = gs_ar0234_read_reg8(sensor, GS_REG_ANTIFLICKER_MODE, &val8); // set power line frequency
		if(ret) break;
		val8 = val8 & 0xFC; // make bit[1..0] zero
		switch(ctrl->val) {
			case V4L2_CID_POWER_LINE_FREQUENCY_DISABLED:
				ret = gs_ar0234_write_reg8(sensor, GS_REG_ANTIFLICKER_MODE, val8 );
				break;
			case V4L2_CID_POWER_LINE_FREQUENCY_50HZ:
				ret = gs_ar0234_write_reg8(sensor, GS_REG_ANTIFLICKER_MODE, val8 | 1);
				if(ret) break;
				ret = gs_ar0234_write_reg8(sensor, GS_REG_ANTIFLICKER_FREQ, 50);
				break;
			case V4L2_CID_POWER_LINE_FREQUENCY_60HZ:
				ret = gs_ar0234_write_reg8(sensor, GS_REG_ANTIFLICKER_MODE, val8 | 1);
				if(ret) break;
				ret = gs_ar0234_write_reg8(sensor, GS_REG_ANTIFLICKER_FREQ, 60);
				break;
			case V4L2_CID_POWER_LINE_FREQUENCY_AUTO:
			default:
				ret = gs_ar0234_write_reg8(sensor, GS_REG_ANTIFLICKER_MODE, val8 | 2);
				break;
		}
		dev_dbg_ratelimited(sd->dev, "%s: set anti flicker  to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_HFLIP:
		// mirror is bit[0], flip is bit[1]. Read vflip value and write both
		ret = gs_ar0234_write_reg8(sensor, GS_REG_MIRROR_FLIP, (sensor->ctrls.vflip->val << 1 | ctrl->val));
		dev_dbg_ratelimited(sd->dev, "%s: set hflip to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_VFLIP:
		// mirror is bit[0], flip is bit[1]. Read hflip value and write both
		ret = gs_ar0234_write_reg8(sensor, GS_REG_MIRROR_FLIP, ctrl->val << 1 | sensor->ctrls.hflip->val);
		dev_dbg_ratelimited(sd->dev, "%s: set hflip to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_SHARPNESS:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_SHARPNESS, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set sharpness to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_COLORFX: // color effect
		switch (ctrl->val) {
			case V4L2_COLORFX_NONE: 		val8=0x00; break;
			case V4L2_COLORFX_BW: 			val8=0x03; break;
			case V4L2_COLORFX_SEPIA:		val8=0x0D; break;
			case V4L2_COLORFX_NEGATIVE: 	val8=0x07; break;
			case V4L2_COLORFX_EMBOSS: 		val8=0x05; break;
			case V4L2_COLORFX_SKETCH: 		val8=0x0F; break;
			case V4L2_COLORFX_SKY_BLUE: 	val8=0x08; break;
			case V4L2_COLORFX_GRASS_GREEN: 	val8=0x09; break;
			case V4L2_COLORFX_SKIN_WHITEN: 	val8=0x00; break; //?
			case V4L2_COLORFX_VIVID: 		val8=0x00; break; // ?
			case V4L2_COLORFX_AQUA: 		val8=0x00; break; // ?
			case V4L2_COLORFX_ART_FREEZE: 	val8=0x11; break; // foggy
			case V4L2_COLORFX_SILHOUETTE: 	val8=0x04; break; //emboss B/W
			case V4L2_COLORFX_SOLARIZATION: val8=0x10; break;
			case V4L2_COLORFX_ANTIQUE: 		val8=0x02; break;
			case V4L2_COLORFX_SET_CBCR: 	val8=0x00; break;
			default: val8 = 0;	break;
		}
		ret = gs_ar0234_write_reg8(sensor, GS_REG_COLORFX, val8);
		dev_dbg_ratelimited(sd->dev, "%s: set colorfx to %d\n", __func__, val8);
		break;
	case V4L2_CID_TEST_PATTERN:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_TESTPATTERN, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set testpattern %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_PAN_ABSOLUTE:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_PAN, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set pan to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_TILT_ABSOLUTE:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_TILT, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set tilt to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_ZOOM_ABSOLUTE:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_ZOOM, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set zoom to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_ZOOM_SPEED:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_ZOOM_SPEED, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set zoom speed to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_NOISE_RED:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_NOISE_RED, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set noise reduction to %d\n", __func__, ctrl->val);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int gs_ar0234_i_cntrl(struct gs_ar0234_dev *sensor)
{
	int ret=0;
	u32 uval32;
	u16 uval;
	u8 uval8;
	union  myunion {
		u16 uval;
		s16 sval;
	} val_t;

	dev_dbg(sensor->dev, "%s: \n", __func__);

	ret = gs_ar0234_read_reg16(sensor, GS_REG_BRIGHTNESS, (short *) &sensor->ctrls.brightness->cur.val);
	//dev_dbg(sensor->dev, "%s: brighness = %x \n", __func__, sensor->ctrls.brightness->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_CONTRAST, (short *)&sensor->ctrls.contrast->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_SATURATION, (short *)&sensor->ctrls.saturation->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_GAMMA, (short *) &sensor->ctrls.gamma->cur);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_SHARPNESS, (short *)&sensor->ctrls.sharpness->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_NOISE_RED, (short *)&sensor->ctrls.noise_red->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_SHARPNESS, (short *) &sensor->ctrls.blc_level->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_WB_TEMPERATURE, (short *) &sensor->ctrls.wb_temp->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_GAIN, (short *) &sensor->ctrls.gain->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_ZOOM, (short *) &sensor->ctrls.zoom->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_ZOOM_SPEED,(u8 *) &sensor->ctrls.zoom_speed->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_PAN, (u8 *) &sensor->ctrls.pan->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_TILT, (u8 *) &sensor->ctrls.tilt->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_TESTPATTERN, (u8 *) &sensor->ctrls.testpattern->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_AE_TARGET, &val_t.uval);
	if (ret < 0) return ret;
	sensor->ctrls.exposure->cur.val = (s32) (((s32) val_t.sval) * 1000 / 256);

	ret = gs_ar0234_read_reg8(sensor, GS_REG_EXPOSURE_MODE, &uval8);
	if (ret < 0) return ret;
	switch(uval8) {
		case 0x0:
			sensor->ctrls.auto_exp->cur.val = V4L2_EXPOSURE_MANUAL;
			break;
		case 0x9:
			sensor->ctrls.auto_exp->cur.val = V4L2_EXPOSURE_SHUTTER_PRIORITY;
			break;
		case 0xC:
			sensor->ctrls.auto_exp->cur.val = V4L2_EXPOSURE_AUTO;
			break;
		default:
			sensor->ctrls.auto_exp->cur.val = V4L2_EXPOSURE_AUTO;
			dev_dbg(sensor->dev, "%s: exposure mode = %x is this correct?\n", __func__, uval8);
			break;
	}

	ret = gs_ar0234_read_reg32(sensor, GS_REG_EXPOSURE_ABS, &uval32);
	if (ret < 0) return ret;
	sensor->ctrls.exposure_absolute->cur.val = uval32/100; // 100us
	//dev_dbg(sensor->dev, "%s: exposure abs = %x %d is this correct?\n", __func__, uval32,sensor->ctrls.exposure_absolute->cur.val);


	ret = gs_ar0234_read_reg8(sensor, GS_REG_BLC_MODE, &uval8);
	if (ret < 0) return ret;
	switch(uval8) {
		case 0x0:
			sensor->ctrls.exposure_metering->cur.val = V4L2_EXPOSURE_METERING_AVERAGE;
			break;
		case 0x1:
			sensor->ctrls.exposure_metering->cur.val = V4L2_EXPOSURE_METERING_CENTER_WEIGHTED;
			break;
		case 0x2:
			sensor->ctrls.exposure_metering->cur.val = V4L2_EXPOSURE_METERING_SPOT;
			break;
		case 0x3:
			sensor->ctrls.exposure_metering->cur.val = V4L2_EXPOSURE_METERING_MATRIX;
			break;
		default:
			sensor->ctrls.exposure_metering->cur.val = V4L2_EXPOSURE_METERING_CENTER_WEIGHTED;
			dev_dbg(sensor->dev, "%s: exposure_metering = %x is this correct?\n", __func__, uval8);
			break;
	}

	ret = gs_ar0234_read_reg8(sensor, GS_REG_WHITEBALANCE, &uval8);
	if (ret < 0) return ret;
	sensor->ctrls.auto_wb->cur.val = uval8 == 0xFF ? 1 : 0;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_WB_TEMPERATURE, &uval);
	if (ret < 0) return ret;
	if(( uval> 2500) && (uval < 3500)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_INCANDESCENT;
	else if(( uval> 2500) && (uval < 3500)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_INCANDESCENT;
	else if(( uval> 3500) && (uval < 4500)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_FLUORESCENT;
	else if(( uval> 4500) && (uval < 5000)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_FLUORESCENT_H;
	else if(( uval> 5000) && (uval < 6000)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_HORIZON;
	else if(( uval> 6000) && (uval < 7000)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_DAYLIGHT;
	//else if(( uval> 5400) && (uval < 5600)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_FLASH;
	else if(( uval> 7000) && (uval < 8000)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_CLOUDY;
	else if(( uval> 8000) && (uval < 10000)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_SHADE;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_MIRROR_FLIP, &uval8);
	if (ret < 0) return ret;
	sensor->ctrls.hflip->cur.val = uval8 & 0x01;
	sensor->ctrls.vflip->cur.val = (uval8>>1) & 0x01;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_ANTIFLICKER_MODE, &uval8);
	if (ret < 0) return ret;
	if((uval8&0x3) == 0)	sensor->ctrls.powerline->cur.val = V4L2_CID_POWER_LINE_FREQUENCY_DISABLED;
	else if((uval8&0x3) == 2)	sensor->ctrls.powerline->cur.val = V4L2_CID_POWER_LINE_FREQUENCY_AUTO;
	else {
		ret = gs_ar0234_read_reg8(sensor, GS_REG_ANTIFLICKER_FREQ, &uval8);
		if (ret < 0) return ret;
		if(uval8 <= 55) sensor->ctrls.powerline->cur.val = V4L2_CID_POWER_LINE_FREQUENCY_50HZ;
		else if(uval8 >= 55) sensor->ctrls.powerline->cur.val = V4L2_CID_POWER_LINE_FREQUENCY_60HZ;
	}

	ret = gs_ar0234_read_reg8(sensor, GS_REG_COLORFX, &uval8);
	if (ret < 0) return ret;
	switch(uval8) {
		case 0x00:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_NONE; break;
		case 0x03:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_BW; break;
		case 0x0D:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_SEPIA; break;
		case 0x07:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_NEGATIVE; break;
		case 0x05:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_EMBOSS; break;
		case 0x0F:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_SKETCH; break;
		case 0x08:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_SKY_BLUE; break;
		case 0x09:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_GRASS_GREEN; break;
		case 0x11:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_ART_FREEZE; break;
		case 0x04:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_SILHOUETTE; break;
		case 0x10:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_SOLARIZATION;	break;
		case 0x02:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_ANTIQUE; break;
		default: sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_NONE; break;
	}


	return ret;
}




static const struct v4l2_ctrl_ops gs_ar0234_ctrl_ops = {
	.g_volatile_ctrl = gs_ar0234_g_volatile_ctrl,
	.s_ctrl = gs_ar0234_s_ctrl,
};

static const char * const gs_ar0234_test_pattern_menu[] = {
	"Disabled",
	"Enabled"
};

#if 0
static u8 gs_ar0234_test_pattern_val[] = {
	0x00,	/* Disabled */
	0x01,	/* Enabled */
};
#endif

static const struct v4l2_ctrl_config zoom_speed = {
        .ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_ZOOM_SPEED,
        .name = "Zoom speed",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = -128,
        .max = 127,
        .step = 1,
		.def = -128,
};


static const struct v4l2_ctrl_config noise_red = {
        .ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_NOISE_RED,
        .name = "Noise reduction",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = -32768,
        .max = 32767,
        .step = 1,
		.def = 0,
};


static int gs_ar0234_init_controls(struct gs_ar0234_dev *sensor)
{
	const struct v4l2_ctrl_ops *ops = &gs_ar0234_ctrl_ops;
	struct gs_ar0234_ctrls *ctrls = &sensor->ctrls;
	struct v4l2_ctrl_handler *hdl = &ctrls->handler;
	int ret;

	v4l2_ctrl_handler_init(hdl, 32);

	dev_dbg_ratelimited(sensor->dev, "%s: \n", __func__);

	/* we can use our own mutex for the ctrl lock */
	hdl->lock = &sensor->lock;

	/* Clock related controls */
	// ctrls->pixel_rate = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_PIXEL_RATE, 0, INT_MAX, 1, gs_ar0234_calc_pixel_rate(sensor));

	/* Auto/manual white balance */
	ctrls->auto_wb = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_AUTO_WHITE_BALANCE, 0, 1, 1, 1);
	ctrls->push_to_white = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_DO_WHITE_BALANCE, 0, 1, 1, 0);
	ctrls->wb_temp = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_WHITE_BALANCE_TEMPERATURE , 0, 0xFFFF, 1, 6500);
	ctrls->wb_preset = v4l2_ctrl_new_std_menu(hdl, ops, V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE, V4L2_WHITE_BALANCE_SHADE, 0x3, V4L2_WHITE_BALANCE_DAYLIGHT);

	/* auto/manual exposure*/
	ctrls->auto_exp = v4l2_ctrl_new_std_menu(hdl, ops, V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_APERTURE_PRIORITY, 0, V4L2_EXPOSURE_AUTO);
	ctrls->brightness = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_BRIGHTNESS, -32768, 32767, 1, 0);
	//ctrls->brightness->flags |= V4L2_CTRL_FLAG_VOLATILE;
	ctrls->exposure_absolute = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_EXPOSURE_ABSOLUTE , 0, 0xFFFF, 1, 333);
	ctrls->exposure = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_EXPOSURE , -32768,  32767, 1, -2200);
	ctrls->gain = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_GAIN , 0x100,  32767, 1, 0x100);
	ctrls->exposure_metering = v4l2_ctrl_new_std_menu(hdl, ops, V4L2_CID_EXPOSURE_METERING, V4L2_EXPOSURE_METERING_MATRIX, 0, V4L2_EXPOSURE_METERING_CENTER_WEIGHTED);
	ctrls->blc_level = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_BACKLIGHT_COMPENSATION , 0,  0x80, 1, 0);

	/* Auto/manual gain */
	// ctrls->auto_gain = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_AUTOGAIN, 0, 1, 1, 1);

	/* basic */
	ctrls->saturation = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_SATURATION, 0, 0x2000, 1, 0x1000);
	ctrls->contrast = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_CONTRAST, -32768, 32767, 1, 0);
	ctrls->hflip = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_HFLIP, 0, 1, 1, 0);
	ctrls->vflip = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_VFLIP, 0, 1, 1, 0);
	ctrls->sharpness = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_SHARPNESS, -32768, 32767, 1, 0);
	ctrls->gamma = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_GAMMA, 0, 0x7FFF, 1, 0);
	ctrls->noise_red = v4l2_ctrl_new_custom(hdl, &noise_red, NULL);

	/* zoom/pan/tilt */
	ctrls->zoom = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_ZOOM_ABSOLUTE, 0, 0x7FFF, 1, 0x0100);
	ctrls->pan = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_PAN_ABSOLUTE, 0, 0x80, 1, 0x40);
	ctrls->tilt = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_TILT_ABSOLUTE, 0, 0x80, 1, 0x40);
	ctrls->zoom_speed = v4l2_ctrl_new_custom(hdl, &zoom_speed, NULL);


	/* anti flicker */
	ctrls->powerline = v4l2_ctrl_new_std_menu(hdl, ops, V4L2_CID_POWER_LINE_FREQUENCY, V4L2_CID_POWER_LINE_FREQUENCY_AUTO, 0, V4L2_CID_POWER_LINE_FREQUENCY_AUTO);

	/* test pattern */
	ctrls->testpattern = v4l2_ctrl_new_std_menu_items(hdl, ops, V4L2_CID_TEST_PATTERN, ARRAY_SIZE(gs_ar0234_test_pattern_menu) - 1, 0, 0, gs_ar0234_test_pattern_menu);

	/* effects */
	ctrls->colorfx = v4l2_ctrl_new_std_menu(hdl, ops, V4L2_CID_COLORFX, V4L2_COLORFX_SET_CBCR, 0, V4L2_COLORFX_NONE);

	if (hdl->error) {
		ret = hdl->error;
		dev_err(sensor->dev, "%s: error: %d\n", __func__, ret);
		goto free_ctrls;
	}

	// ctrls->pixel_rate->flags |= V4L2_CTRL_FLAG_READ_ONLY;
	// ctrls->gain->flags |= V4L2_CTRL_FLAG_VOLATILE;
	// ctrls->exposure->flags |= V4L2_CTRL_FLAG_VOLATILE;

	// v4l2_ctrl_auto_cluster(3, &ctrls->auto_wb, 0, false);
	// v4l2_ctrl_auto_cluster(2, &ctrls->auto_gain, 0, true);
	// v4l2_ctrl_auto_cluster(2, &ctrls->auto_exp, 1, true);

	sensor->sd.ctrl_handler = hdl;
	return 0;

free_ctrls:
	v4l2_ctrl_handler_free(hdl);
	return ret;
}

static int ops_enum_frame_size(struct v4l2_subdev *sub_dev, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_frame_size_enum *fse)
{
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sub_dev);
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
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sub_dev);
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
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sub_dev);

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
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sub_dev);

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

static int gs_ar0234_enum_mbus_code(struct v4l2_subdev *sub_dev, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_mbus_code_enum *code)
{
	int ret = 0;
	dev_dbg_ratelimited(sub_dev->dev, "%s: \n", __func__);

	if ((code->pad >= NUM_PADS))
		return -EINVAL;

	switch (code->index) {
		case 	GS_CF_YUV422:
		case 	GS_CF_YUV420:
		case 	GS_CF_YUV400:
		case 	GS_CF_YUV422BT:
		case 	GS_CF_YUV420BT:
		case 	GS_CF_YUV400BT:
			code->code = MEDIA_BUS_FMT_YUYV8_1X16;
			break;
		case 	GS_CF_RGB_888:
			code->code = MEDIA_BUS_FMT_RGB888_3X8;
			break;
		case 	GS_CF_RGB_565:
			code->code = MEDIA_BUS_FMT_RGB565_2X8_LE;
			break;
		case 	GS_CF_RGB_555:
			code->code = MEDIA_BUS_FMT_RGB555_2X8_PADHI_LE;
			break;
		case 	GS_CF_JPEG422:
		case 	GS_CF_JPEG420:
		case 	GS_CF_JPEG422EX:
		case 	GS_CF_JPEG420EX:
			code->code = MEDIA_BUS_FMT_JPEG_1X8;
			break;
		case 	GS_CF_BAYER_16:
			code->code = MEDIA_BUS_FMT_SRGGB16_1X16;
			break;
		case 	GS_CF_BAYER_12:
			code->code = MEDIA_BUS_FMT_SRGGB12_1X12;
			break;
		case 	GS_CF_BAYER_10:
			code->code = MEDIA_BUS_FMT_SRGGB10_1X10;
			break;
		case 	GS_CF_BAYER_8:
			code->code = MEDIA_BUS_FMT_SRGGB8_1X8;
			break;
		default:
			ret = -EINVAL;
			break;
	}
	dev_dbg_ratelimited(sub_dev->dev, "%s: code->code = 0x%08x\n", __func__, code->code);
	return ret;
}

static int gs_ar0234_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sd);
	int ret = 0;

	if (sensor->ep.bus_type != V4L2_MBUS_CSI2_DPHY){
		dev_err(sensor->dev, "endpoint bus_type not supported: %d\n", sensor->ep.bus_type);
		return -EINVAL;
	}
#if 1
	mutex_lock(&sensor->lock);

	// turn off mipi?
	gs_ar0234_write_reg8(sensor, 0xE1, 0x02); // format change state
	// set rres, fixed formats reg 0x10,
	gs_ar0234_write_reg8(sensor, 0x10, sensor->mode->frame_format_code);
	// set fr reg- 0x16 (16b = 8b,8b [fraction)]) = 60,50,30,25 or any int
	gs_ar0234_write_reg16(sensor, 0x16, ((u16)(sensor->mode->framerate) << 8));
	//sensor->fmt.
	//  set format type
	//turn on mipi
	gs_ar0234_write_reg8(sensor, 0xE1, 0x03); // format change state

	mutex_unlock(&sensor->lock);
#endif
	if (enable)
		pr_debug("%s: Starting stream at WxH@fps=%dx%d@%d\n", __func__, sensor->mode->width, sensor->mode->height, sensor->mode->framerate);
	else
		pr_debug("%s: Stopping stream \n", __func__);

	return ret;
}

static const struct v4l2_subdev_core_ops gs_ar0234_core_ops = {
	.s_power = gs_ar0234_s_power,
	.log_status = v4l2_ctrl_subdev_log_status,
	.subscribe_event = v4l2_ctrl_subdev_subscribe_event,
	.unsubscribe_event = v4l2_event_subdev_unsubscribe,
};

static const struct v4l2_subdev_video_ops gs_ar0234_video_ops = {
	.g_frame_interval = ops_get_frame_interval,
	.s_frame_interval = ops_set_frame_interval,
	.s_stream = gs_ar0234_s_stream,
};

static const struct v4l2_subdev_pad_ops gs_ar0234_pad_ops = {
	.enum_mbus_code = gs_ar0234_enum_mbus_code,
	.get_fmt = ops_get_fmt,
	.set_fmt = gs_ar0234_set_fmt,
	.enum_frame_size = ops_enum_frame_size,
	.enum_frame_interval = ops_enum_frame_interval,
};

static const struct v4l2_subdev_ops gs_ar0234_subdev_ops = {
	.core = &gs_ar0234_core_ops,
	.video = &gs_ar0234_video_ops,
	.pad = &gs_ar0234_pad_ops,
};

static int gs_ar0234_link_setup(struct media_entity *entity, const struct media_pad *local, const struct media_pad *remote, u32 flags)
{
	return 0;
}

static const struct media_entity_operations gs_ar0234_sd_media_ops = {
	.link_setup = gs_ar0234_link_setup,
};

static const struct regmap_config sensor_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.cache_type = REGCACHE_NONE,
};

static int gs_ar0234_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct fwnode_handle *endpoint;
	struct gs_ar0234_dev *sensor;
	struct v4l2_mbus_framefmt *fmt;
	int ret;
	//unsigned int id_code;

	pr_debug("-->%s gs_ar0234 Probe start\n",__func__);

	sensor = devm_kzalloc(dev, sizeof(*sensor), GFP_KERNEL);
	if (!sensor)
		return -ENOMEM;

	sensor->dev = dev;

	// default init sequence initialize sensor to 1080p30 YUV422 UYVY
	fmt = &sensor->fmt;
	fmt->code = MEDIA_BUS_FMT_UYVY8_1X16; // MEDIA_BUS_FMT_UYVY8_2X8
	fmt->colorspace = V4L2_COLORSPACE_SRGB;
	fmt->ycbcr_enc = V4L2_MAP_YCBCR_ENC_DEFAULT(fmt->colorspace);
	fmt->quantization = V4L2_QUANTIZATION_FULL_RANGE;
	fmt->xfer_func = V4L2_MAP_XFER_FUNC_DEFAULT(fmt->colorspace);
	fmt->width = 1280;
	fmt->height = 720;
	fmt->field = V4L2_FIELD_NONE;
	sensor->framerate = 30;
	sensor->mode = &sensor_res_list[0];
	sensor->mbus_num = GS_CF_YUV422;

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
	sensor->i2c_client = client;
	// ret = regmap_read(sensor->regmap, 0x1, &id_code);
	// if (ret) {
	// 	/* If we can't read the ID, it may be that the FPGA hasn't loaded yet after releasing reset. */
	// 	dev_dbg(dev, "Could not read device-id. trying again\n");
	// 	return -EPROBE_DEFER;
	// }
	// else {
	// 	dev_info(dev, "Got device-id %02x\n", id_code);
	// }

/*
	u16 format_x = 0;
	gs_ar0234_read_reg16(sensor, 0x12, &format_x);
	dev_info(dev, "got format-x value: %d", format_x);
*/
	v4l2_i2c_subdev_init(&sensor->sd, client, &gs_ar0234_subdev_ops);

	sensor->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS | V4L2_SUBDEV_FL_HAS_DEVNODE;
	sensor->sd.dev = &client->dev;
	sensor->pad.flags = MEDIA_PAD_FL_SOURCE;
	sensor->sd.entity.ops = &gs_ar0234_sd_media_ops;
	sensor->sd.entity.function = MEDIA_ENT_F_CAM_SENSOR;
	ret = media_entity_pads_init(&sensor->sd.entity, 1, &sensor->pad);
	if (ret)
		return ret;

	mutex_init(&sensor->lock);

	ret = gs_ar0234_init_controls(sensor);
	if (ret)
		goto entity_cleanup;

	//turn off
	///////////////////////////////gs_ar0234_power(sensor, 0);

	ret = v4l2_async_register_subdev_sensor(&sensor->sd);
	if (ret)
		goto entity_cleanup;


	// read register values from Sensor
	ret = gs_ar0234_i_cntrl(sensor);
	if (ret)
		goto entity_cleanup;


	pr_debug("<--%s gs_ar0234 Probe end successful, return\n",__func__);
	return 0;


	ret = v4l2_async_register_subdev_sensor(&sensor->sd);
	if (ret)
		goto free_ctrls;

	return 0;

free_ctrls:
	v4l2_ctrl_handler_free(&sensor->ctrls.handler);
entity_cleanup:
	pr_debug("---%s gs_ar0234 ERR entity_cleanup\n",__func__);
	media_entity_cleanup(&sensor->sd.entity);
	mutex_destroy(&sensor->lock);
	return ret;
}

static void gs_ar0234_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sd);

	v4l2_async_unregister_subdev(&sensor->sd);
	media_entity_cleanup(&sensor->sd.entity);
	mutex_destroy(&sensor->lock);
}

static const struct i2c_device_id gs_ar0234_id[] = {
	{"gs_ar0234", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, gs_ar0234_id);

static const struct of_device_id gs_ar0234_dt_ids[] = {
	{ .compatible = "scailx,gs_AR0234" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, gs_ar0234_dt_ids);

static struct i2c_driver gs_ar0234_i2c_driver = {
	.driver = {
		.name  = "gs_ar0234",
		.of_match_table	= gs_ar0234_dt_ids,
	},
	.id_table = gs_ar0234_id,
	.probe_new = gs_ar0234_probe,
	.remove   = gs_ar0234_remove,
};

module_i2c_driver(gs_ar0234_i2c_driver);

MODULE_DESCRIPTION("gs_ar0234 MIPI Camera Subdev Driver");
MODULE_LICENSE("GPL");