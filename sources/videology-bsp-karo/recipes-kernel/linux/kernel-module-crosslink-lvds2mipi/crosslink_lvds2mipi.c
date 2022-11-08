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
#include <media/v4l2-async.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-subdev.h>

enum crosslink_mode_id {
	// crosslink_MODE_720P,
	crosslink_MODE_1080P,
	crosslink_NUM_MODES,
};

enum crosslink_frame_rate {
	crosslink_25_FPS = 0,
	// crosslink_30_FPS,
	// crosslink_60_FPS,
	crosslink_NUM_FRAMERATES,
};

struct crosslink_pixfmt {
	u32 code;
	u32 colorspace;
};

static const struct crosslink_pixfmt crosslink_formats[] = {
	{ MEDIA_BUS_FMT_UYVY8_2X8, V4L2_COLORSPACE_SRGB, },
};

static const int crosslink_framerates[] = {
	[crosslink_25_FPS] = 25,
	// [crosslink_30_FPS] = 30,
	// [crosslink_60_FPS] = 60,
};

struct crosslink_mode_info {
	enum crosslink_mode_id id;
	u32 hact;
	u32 vact;
	u32 max_fps;
};

struct crosslink_ctrls {
	struct v4l2_ctrl_handler handler;
	struct v4l2_ctrl *pixel_rate;
	struct {
		struct v4l2_ctrl *auto_exp;
		struct v4l2_ctrl *exposure;
	};
	struct {
		struct v4l2_ctrl *auto_wb;
		struct v4l2_ctrl *blue_balance;
		struct v4l2_ctrl *red_balance;
	};
	struct {
		struct v4l2_ctrl *auto_gain;
		struct v4l2_ctrl *gain;
	};
	struct v4l2_ctrl *brightness;
	struct v4l2_ctrl *light_freq;
	struct v4l2_ctrl *saturation;
	struct v4l2_ctrl *contrast;
	struct v4l2_ctrl *hue;
	struct v4l2_ctrl *test_pattern;
	struct v4l2_ctrl *hflip;
	struct v4l2_ctrl *vflip;
};

struct crosslink_dev {
	struct device *dev;
	struct regmap *regmap;

	struct v4l2_subdev sd;
	struct media_pad pad;
	struct v4l2_fwnode_endpoint ep; /* the parsed DT endpoint info */

	int reset_gpio;

	/* lock to protect all members below */
	struct mutex lock;

	int power_count;

	struct v4l2_mbus_framefmt fmt;
	bool pending_fmt_change;

	const struct crosslink_mode_info *current_mode;
	const struct crosslink_mode_info *last_mode;
	enum crosslink_frame_rate current_fr;
	struct v4l2_fract frame_interval;

	struct crosslink_ctrls ctrls;

	bool pending_mode_change;
	bool streaming;
};

static inline struct crosslink_dev *to_crosslink_dev(struct v4l2_subdev *sd)
{
	return container_of(sd, struct crosslink_dev, sd);
}

static inline struct v4l2_subdev *ctrl_to_sd(struct v4l2_ctrl *ctrl)
{
	return &container_of(ctrl->handler, struct crosslink_dev, ctrls.handler)->sd;
}

static const struct crosslink_mode_info
crosslink_mode_data[crosslink_NUM_MODES] = {
	// {crosslink_MODE_720P, 1280, 720, crosslink_60_FPS},
	{crosslink_MODE_1080P, 1920, 1080, crosslink_25_FPS},
};

// static int crosslink_write_reg(struct crosslink_dev *sensor, u16 reg, u8 val)
// {
// 	struct i2c_client *client = sensor->i2c_client;
// 	struct i2c_msg msg;
// 	u8 buf[3];
// 	int ret;

// 	buf[0] = reg >> 8;
// 	buf[1] = reg & 0xff;
// 	buf[2] = val;

// 	msg.addr = client->addr;
// 	msg.flags = client->flags;
// 	msg.buf = buf;
// 	msg.len = sizeof(buf);

// 	ret = i2c_transfer(client->adapter, &msg, 1);
// 	if (ret < 0) {
// 		dev_err(&client->dev, "%s: error: reg=%x, val=%x\n", __func__, reg, val);
// //HH		return ret;
// 	}

// 	return 0;
// }

// static int crosslink_read_reg(struct crosslink_dev *sensor, u16 reg, u8 *val)
// {
// 	struct i2c_client *client = sensor->i2c_client;
// 	struct i2c_msg msg[2];
// 	u8 buf[2];
// 	int ret;

// 	buf[0] = reg >> 8;
// 	buf[1] = reg & 0xff;

// 	msg[0].addr = client->addr;
// 	msg[0].flags = client->flags;
// 	msg[0].buf = buf;
// 	msg[0].len = sizeof(buf);

// 	msg[1].addr = client->addr;
// 	msg[1].flags = client->flags | I2C_M_RD;
// 	msg[1].buf = buf;
// 	msg[1].len = 1;

// 	ret = i2c_transfer(client->adapter, msg, 2);
// 	if (ret < 0) {
// 		dev_err(&client->dev, "%s: error: reg=%x\n", __func__, reg);
// //HH		return ret;
// 	}

// 	*val = buf[0];
// 	return 0;
// }

static int crosslink_check_valid_mode(struct crosslink_dev *sensor, const struct crosslink_mode_info *mode, enum crosslink_frame_rate rate)
{
	int ret = 0;

	if (sensor->ep.bus_type == V4L2_MBUS_CSI2_DPHY) {
		switch (mode->id) {
			// case crosslink_MODE_720P:
			// 	if ((rate != crosslink_25_FPS) &&
			// 	    (rate != crosslink_30_FPS))
			// 		ret = -EINVAL;
			// 	break;
			case crosslink_MODE_1080P:
				pr_debug("%s: case crosslink_MODE_1080P \n", __func__);
				if ((rate != crosslink_25_FPS)){
					pr_debug("%s: : rate != crosslink_25_FPS\n", __func__);
					ret = -EINVAL;
				}
				break;
			default:
				dev_err(sensor->dev, "Invalid mode (%d)\n", mode->id);
				ret = -EINVAL;
		}
	}
	else
	{
		pr_debug("%s: bus type not V4L2_MBUS_CSI2_DPHY\n", __func__);
		ret = -EINVAL;

	}
	return ret;
}

/* read exposure, in number of line periods */
static int crosslink_get_exposure(struct crosslink_dev *sensor)
{
	int ret = 10;
	pr_debug("enter %s\n", __func__);
	return ret;
}

static int crosslink_get_gain(struct crosslink_dev *sensor)
{
	return 10;
}

static const struct crosslink_mode_info *
crosslink_find_mode(struct crosslink_dev *sensor, enum crosslink_frame_rate fr, int width, int height, bool nearest)
{
	const struct crosslink_mode_info *mode;
	pr_debug("%s: enter \n", __func__);

	mode = v4l2_find_nearest_size(crosslink_mode_data, ARRAY_SIZE(crosslink_mode_data), hact, vact, width, height);

	if (!mode ||
	    (!nearest && (mode->hact != width || mode->vact != height))) {
		return NULL;
	}
	pr_debug("%s: end \n", __func__);
	return mode;
}

static int crosslink_set_mode(struct crosslink_dev *sensor)
{
	const struct crosslink_mode_info *mode = sensor->current_mode;

	pr_debug("enter %s\n", __func__);

	sensor->pending_mode_change = false;
	sensor->last_mode = mode;

	return 0;
}


static int crosslink_set_framefmt(struct crosslink_dev *sensor, struct v4l2_mbus_framefmt *format);

static void crosslink_power(struct crosslink_dev *sensor, bool enable)
{
	pr_debug("enter %s\n", __func__);
	if (sensor->reset_gpio < 0)
		return;

	pr_debug("%s: setting reset pin: %s\n", __func__, enable ? "ON" : "OFF");

	gpio_set_value(sensor->reset_gpio, enable ? 1 : 0);
}

/* --------------- Subdev Operations --------------- */

static int crosslink_s_power(struct v4l2_subdev *sd, int on)
{
	struct crosslink_dev *sensor = to_crosslink_dev(sd);
	int ret = 0;

	mutex_lock(&sensor->lock);

	pr_debug("%s: set power callback %d\n", __func__, on);
	// crosslink_power(sensor, (bool)on);

	mutex_unlock(&sensor->lock);
	return ret;
}

static int crosslink_try_frame_interval(struct crosslink_dev *sensor, struct v4l2_fract *fi, u32 width, u32 height)
{
	const struct crosslink_mode_info *mode;
	enum crosslink_frame_rate rate = crosslink_25_FPS;
	int minfps, maxfps, best_fps, fps;
	int i;

	pr_debug("%s: enter \n", __func__);

	minfps = crosslink_framerates[crosslink_25_FPS];
	maxfps = crosslink_framerates[crosslink_25_FPS];

	if (fi->numerator == 0) {
		fi->denominator = maxfps;
		fi->numerator = 1;
		rate = crosslink_25_FPS;
		goto find_mode;
	}

	fps = clamp_val(DIV_ROUND_CLOSEST(fi->denominator, fi->numerator), minfps, maxfps);

	best_fps = minfps;
	for (i = 0; i < ARRAY_SIZE(crosslink_framerates); i++) {
		int curr_fps = crosslink_framerates[i];

		if (abs(curr_fps - fps) < abs(best_fps - fps)) {
			best_fps = curr_fps;
			rate = i;
		}
	}

	fi->numerator = 1;
	fi->denominator = best_fps;

find_mode:
	mode = crosslink_find_mode(sensor, rate, width, height, false);
	pr_debug("%s: end \n", __func__);
	return mode ? rate : -EINVAL;
}

static int crosslink_get_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_format *format)
{
	struct crosslink_dev *sensor = to_crosslink_dev(sd);
	struct v4l2_mbus_framefmt *fmt;

	pr_debug("%s: enter \n", __func__);

	if (format->pad != 0)
		return -EINVAL;

	mutex_lock(&sensor->lock);

	if (format->which == V4L2_SUBDEV_FORMAT_TRY)
		fmt = v4l2_subdev_get_try_format(&sensor->sd, sd_state, format->pad);
	else
		fmt = &sensor->fmt;

	fmt->reserved[1] = (sensor->current_fr == crosslink_25_FPS) ? 25 : 30;
	format->format = *fmt;

	mutex_unlock(&sensor->lock);
	pr_debug("%s: exit \n", __func__);
	return 0;
}

static int crosslink_try_fmt_internal(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *fmt, enum crosslink_frame_rate fr, const struct crosslink_mode_info **new_mode)
{
	struct crosslink_dev *sensor = to_crosslink_dev(sd);
	const struct crosslink_mode_info *mode;
	int i;

	pr_debug("%s: enter \n", __func__);

	mode = crosslink_find_mode(sensor, fr, fmt->width, fmt->height, true);
	if (!mode)
		return -EINVAL;
	fmt->width = mode->hact;
	fmt->height = mode->vact;
	memset(fmt->reserved, 0, sizeof(fmt->reserved));

	if (new_mode)
		*new_mode = mode;

	for (i = 0; i < ARRAY_SIZE(crosslink_formats); i++)
		if (crosslink_formats[i].code == fmt->code)
			break;
	if (i >= ARRAY_SIZE(crosslink_formats))
		i = 0;

	fmt->code = crosslink_formats[i].code;
	fmt->colorspace = crosslink_formats[i].colorspace;
	fmt->ycbcr_enc = V4L2_MAP_YCBCR_ENC_DEFAULT(fmt->colorspace);
	fmt->quantization = V4L2_QUANTIZATION_FULL_RANGE;
	fmt->xfer_func = V4L2_MAP_XFER_FUNC_DEFAULT(fmt->colorspace);

	pr_debug("%s: exit \n", __func__);
	return 0;
}

static int crosslink_set_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_format *format)
{
	struct crosslink_dev *sensor = to_crosslink_dev(sd);
	const struct crosslink_mode_info *new_mode;
	struct v4l2_mbus_framefmt *mbus_fmt = &format->format;
	struct v4l2_mbus_framefmt *fmt;
	int ret;

	pr_debug("%s: enter \n", __func__);

	if (format->pad != 0)
		return -EINVAL;

	mutex_lock(&sensor->lock);

	ret = crosslink_try_fmt_internal(sd, mbus_fmt, sensor->current_fr, &new_mode);
	if (ret){
	pr_debug("%s: crosslink_try_fmt_internal ERROR \n", __func__);
		goto out;
		}

	if (format->which == V4L2_SUBDEV_FORMAT_TRY)
		fmt = v4l2_subdev_get_try_format(sd, sd_state, 0);
	else
		fmt = &sensor->fmt;

	*fmt = *mbus_fmt;

	if (new_mode != sensor->current_mode) {
		sensor->current_mode = new_mode;
		sensor->pending_mode_change = true;
	}
	if (mbus_fmt->code != sensor->fmt.code)
		sensor->pending_fmt_change = true;

	__v4l2_ctrl_s_ctrl_int64(sensor->ctrls.pixel_rate, 297000000*2);

	if (sensor->pending_mode_change || sensor->pending_fmt_change)
		sensor->fmt = *mbus_fmt;

	pr_debug("%s: sensor->ep.bus_type =%d\n", __func__, sensor->ep.bus_type);
	pr_debug("%s: sensor->ep.bus      =%p\n", __func__, &sensor->ep.bus);
	pr_debug("%s: sensor->fmt->height =%d\n", __func__, mbus_fmt->height);
	pr_debug("%s: sensor->fmt->width  =%d\n", __func__, mbus_fmt->width);
	pr_debug("%s: end \n", __func__);
out:
	mutex_unlock(&sensor->lock);
	return ret;
}

static int crosslink_set_framefmt(struct crosslink_dev *sensor, struct v4l2_mbus_framefmt *format)
{
	pr_debug("enter %s\n", __func__);

	return 0;
}

/*
 * Sensor Controls.
 */
static int crosslink_set_ctrl_gain(struct crosslink_dev *sensor, bool auto_gain)
{
	int ret = 0;
	pr_debug("enter %s\n", __func__);
	return ret;
}

static int crosslink_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
	struct crosslink_dev *sensor = to_crosslink_dev(sd);
	int val;

	pr_debug("%s: enter \n", __func__);
	/* v4l2_ctrl_lock() locks our own mutex */

	switch (ctrl->id) {
	case V4L2_CID_AUTOGAIN:
		val = crosslink_get_gain(sensor);
		if (val < 0)
			return val;
		sensor->ctrls.gain->val = val;
		break;
	case V4L2_CID_EXPOSURE_AUTO:
		val = crosslink_get_exposure(sensor);
		if (val < 0)
			return val;
		sensor->ctrls.exposure->val = val;
		break;
	}

	return 0;
}

static int crosslink_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
	struct crosslink_dev *sensor = to_crosslink_dev(sd);
	int ret;

	pr_debug("%s: enter \n", __func__);

	/* v4l2_ctrl_lock() locks our own mutex */

	/*
	 * If the device is not powered up by the host driver do
	 * not apply any controls to H/W at this time. Instead
	 * the controls will be restored right after power-up.
	 */
	if (sensor->power_count == 0)
		return 0;

	switch (ctrl->id) {
	case V4L2_CID_AUTOGAIN:
		ret = crosslink_set_ctrl_gain(sensor, ctrl->val);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	pr_debug("%s: exit \n", __func__);
	return ret;
}

static const struct v4l2_ctrl_ops crosslink_ctrl_ops = {
	.g_volatile_ctrl = crosslink_g_volatile_ctrl,
	.s_ctrl = crosslink_s_ctrl,
};

static int crosslink_init_controls(struct crosslink_dev *sensor)
{
	const struct v4l2_ctrl_ops *ops = &crosslink_ctrl_ops;
	struct crosslink_ctrls *ctrls = &sensor->ctrls;
	struct v4l2_ctrl_handler *hdl = &ctrls->handler;
	int ret;

	pr_debug("%s: enter \n", __func__);

	v4l2_ctrl_handler_init(hdl, 32);

	/* we can use our own mutex for the ctrl lock */
	hdl->lock = &sensor->lock;

	/* Clock related controls */
	ctrls->pixel_rate = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_PIXEL_RATE, 0, INT_MAX, 1, 297000000);

	ctrls->exposure = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_EXPOSURE, 0, 65535, 1, 0);
	ctrls->gain = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_GAIN, 0, 1023, 1, 0);

	if (hdl->error) {
		ret = hdl->error;
		goto free_ctrls;
	}

	ctrls->pixel_rate->flags |= V4L2_CTRL_FLAG_READ_ONLY;
	ctrls->gain->flags |= V4L2_CTRL_FLAG_VOLATILE;
	ctrls->exposure->flags |= V4L2_CTRL_FLAG_VOLATILE;

	sensor->sd.ctrl_handler = hdl;
	pr_debug("%s: ok\n", __func__);
	return 0;

free_ctrls:
	pr_debug("%s: error hdl->error returned from v4l2_ctrl_new_std_menu(..)\n", __func__);
	v4l2_ctrl_handler_free(hdl);
	return ret;
}

static int crosslink_enum_frame_size(struct v4l2_subdev *sd, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_frame_size_enum *fse)
{
	if (fse->pad != 0)
		return -EINVAL;
	if (fse->index >= crosslink_NUM_MODES)
		return -EINVAL;

	fse->min_width =
		crosslink_mode_data[fse->index].hact;
	fse->max_width = fse->min_width;
	fse->min_height =
		crosslink_mode_data[fse->index].vact;
	fse->max_height = fse->min_height;

	return 0;
}

static int crosslink_enum_frame_interval(struct v4l2_subdev *sd, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_frame_interval_enum *fie)
{
	struct crosslink_dev *sensor = to_crosslink_dev(sd);
	int i, j, count;

	// pr_debug("enter %s\n", __func__);

	if (fie->pad != 0)
		return -EINVAL;
	if (fie->index >= crosslink_NUM_FRAMERATES)
		return -EINVAL;

	if (fie->width == 0 || fie->height == 0 || fie->code == 0) {
		pr_warn("Please assign pixel format, width and height.\n");
		return -EINVAL;
	}

	fie->interval.numerator = 1;

	count = 0;
	for (i = 0; i < crosslink_NUM_FRAMERATES; i++) {
		for (j = 0; j < crosslink_NUM_MODES; j++) {
			if (fie->width  == crosslink_mode_data[j].hact &&
			    fie->height == crosslink_mode_data[j].vact &&
			    !crosslink_check_valid_mode(sensor, &crosslink_mode_data[j], i))
				count++;

			if (fie->index == (count - 1)) {
				fie->interval.denominator = crosslink_framerates[i];
				return 0;
			}
		}
	}

	return -EINVAL;
}

static int crosslink_g_frame_interval(struct v4l2_subdev *sd, struct v4l2_subdev_frame_interval *fi)
{
	struct crosslink_dev *sensor = to_crosslink_dev(sd);

	mutex_lock(&sensor->lock);
	fi->interval = sensor->frame_interval;
	mutex_unlock(&sensor->lock);

	return 0;
}

static int crosslink_s_frame_interval(struct v4l2_subdev *sd, struct v4l2_subdev_frame_interval *fi)
{
	struct crosslink_dev *sensor = to_crosslink_dev(sd);
	const struct crosslink_mode_info *mode;
	int frame_rate, ret = 0;

	if (fi->pad != 0)
		return -EINVAL;

	mutex_lock(&sensor->lock);

	mode = sensor->current_mode;

	frame_rate = crosslink_try_frame_interval(sensor, &fi->interval, mode->hact, mode->vact);
	if (frame_rate < 0) {
		/* Always return a valid frame interval value */
		fi->interval = sensor->frame_interval;
		goto out;
	}

	mode = crosslink_find_mode(sensor, frame_rate, mode->hact, mode->vact, true);
	if (!mode) {
		ret = -EINVAL;
		goto out;
	}

	if (mode != sensor->current_mode ||
	    frame_rate != sensor->current_fr) {
		sensor->current_fr = frame_rate;
		sensor->frame_interval = fi->interval;
		sensor->current_mode = mode;
		sensor->pending_mode_change = true;

		__v4l2_ctrl_s_ctrl_int64(sensor->ctrls.pixel_rate, 297000000*2);
	}
out:
	mutex_unlock(&sensor->lock);
	return ret;
}

static int crosslink_enum_mbus_code(struct v4l2_subdev *sd, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_mbus_code_enum *code)
{
	if (code->pad != 0)
		return -EINVAL;
	if (code->index >= ARRAY_SIZE(crosslink_formats))
		return -EINVAL;

	code->code = crosslink_formats[code->index].code;
	pr_debug("%s: code->code = 0x%4x\n", __func__, code->code);
	return 0;
}

static int crosslink_soft_reset(struct crosslink_dev *sensor, bool enable)
{
	int ret;

	ret = regmap_write(sensor->regmap, 0x2, enable ? 0 : 1);
	if (ret < 0)
		dev_err(sensor->dev, "could not send soft-reset %d\n", (int)enable);
	else
		dev_dbg(sensor->dev, "wrote to crosslink soft-reset reg -> %d\n", enable ? 0 : 1);
	return ret;
}

static int crosslink_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct crosslink_dev *sensor = to_crosslink_dev(sd);

	int ret = 0;

	pr_debug(">>>> %s - start: sensor->ep.bus_type=%d \n", __func__, sensor->ep.bus_type);

	mutex_lock(&sensor->lock);

	if (sensor->streaming == !enable) {
		pr_debug("%s: sensor->streaming was= %d\n", __func__, sensor->streaming);
		ret = crosslink_check_valid_mode(sensor, sensor->current_mode, sensor->current_fr);
		if (ret) {
			pr_debug("%p - %s - Not support WxH@fps=%dx%d@%d\n", sensor->dev, __func__, sensor->current_mode->hact, sensor->current_mode->vact, crosslink_framerates[sensor->current_fr]);
			goto out;
		}
		else
			pr_debug("%s: Support WxH@fps=%dx%d@%d\n", __func__, sensor->current_mode->hact, sensor->current_mode->vact, crosslink_framerates[sensor->current_fr]);

		if (enable && sensor->pending_mode_change) {
			ret = crosslink_set_mode(sensor);
			if (ret){
				pr_debug("%s: crosslink_set_mode failed\n", __func__);
//				goto out;
			} else
				pr_debug("%s: crosslink_set_mode OK\n", __func__);
		}

		if (enable && sensor->pending_fmt_change) {
			ret = crosslink_set_framefmt(sensor, &sensor->fmt);
			if (ret){
			pr_debug("%s: crosslink_set_framefmt failed\n", __func__);
//				goto out;
		}
			sensor->pending_fmt_change = false;
		}else
			pr_debug("%s: crosslink_set_framefmt OK\n", __func__);


		if (sensor->ep.bus_type != V4L2_MBUS_CSI2_DPHY){
			dev_err(sensor->dev, "endpoint bus_type not supported: %d\n", sensor->ep.bus_type);
			ret = -EINVAL;
		}

		if (!ret){
			sensor->streaming = enable;
			pr_debug("<<<<< %s: sensor->streaming success: %s \n", __func__, enable ? "ENABL":"DISABL");
			crosslink_power(sensor, enable);
			// crosslink_soft_reset(sensor, enable);
		}
	}
	else
		pr_debug("<<<<< %s: sensor->streaming already %d \n", __func__, enable);

out:
	mutex_unlock(&sensor->lock);
	if (ret)
		pr_debug("%s: something went wrong. \n", __func__);
	return ret;
}

static const struct v4l2_subdev_core_ops crosslink_core_ops = {
	.s_power = crosslink_s_power,
	.log_status = v4l2_ctrl_subdev_log_status,
	.subscribe_event = v4l2_ctrl_subdev_subscribe_event,
	.unsubscribe_event = v4l2_event_subdev_unsubscribe,
};

static const struct v4l2_subdev_video_ops crosslink_video_ops = {
	.g_frame_interval = crosslink_g_frame_interval,
	.s_frame_interval = crosslink_s_frame_interval,
	.s_stream = crosslink_s_stream,
};

static const struct v4l2_subdev_pad_ops crosslink_pad_ops = {
	.enum_mbus_code = crosslink_enum_mbus_code,
	.get_fmt = crosslink_get_fmt,
	.set_fmt = crosslink_set_fmt,
	.enum_frame_size = crosslink_enum_frame_size,
	.enum_frame_interval = crosslink_enum_frame_interval,
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

	pr_debug("-->%s crosslink Probe start\n",__func__);

	sensor = devm_kzalloc(dev, sizeof(*sensor), GFP_KERNEL);
	if (!sensor)
		return -ENOMEM;

	sensor->dev = dev;

	/*
	 * default init sequence initialize sensor to
	 * YUV422 UYVY VGA@30fps
	 */
	fmt = &sensor->fmt;
	fmt->code = MEDIA_BUS_FMT_UYVY8_2X8;
	fmt->colorspace = V4L2_COLORSPACE_SRGB;
	fmt->ycbcr_enc = V4L2_MAP_YCBCR_ENC_DEFAULT(fmt->colorspace);
	fmt->quantization = V4L2_QUANTIZATION_FULL_RANGE;
	fmt->xfer_func = V4L2_MAP_XFER_FUNC_DEFAULT(fmt->colorspace);
	fmt->width = 1920;
	fmt->height = 1080;
	fmt->field = V4L2_FIELD_NONE;
	sensor->frame_interval.numerator = 1;
	sensor->frame_interval.denominator = crosslink_framerates[crosslink_25_FPS];
	sensor->current_fr = crosslink_25_FPS;
	sensor->current_mode = &crosslink_mode_data[crosslink_MODE_1080P];
	sensor->last_mode = sensor->current_mode;

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

	/* request reset pin */
	sensor->reset_gpio = of_get_named_gpio(dev->of_node, "reset-gpios", 0);
	if (!gpio_is_valid(sensor->reset_gpio))
		dev_warn(dev, "No crosslink reset pin available");
	else {
		dev_dbg(dev, "got GPIO, requesting...");
		ret = devm_gpio_request_one(dev, sensor->reset_gpio, GPIOF_OUT_INIT_LOW, "ov5640_mipi_reset");
		if (ret < 0) {
			dev_warn(dev, "Failed to set reset pin\n");
			// return retval;
		} else
			dev_dbg(dev, "successfully set RESET-GPIO");
	}

// 	sensor->reset_gpio = devm_gpiod_get(dev, "reset-gpios", GPIOD_OUT_HIGH);
// 	if (IS_ERR(sensor->reset_gpio)){
// 		pr_debug("%s: sensor->reset_gpio not found\n", __func__);
// //		return PTR_ERR(sensor->reset_gpio);
// 	} else {
// 		pr_debug("%s: found reset_gpio.\n", __func__);
// 	}

	sensor->regmap = devm_regmap_init_i2c(client, &sensor_regmap_config);
	if (IS_ERR(sensor->regmap)) {
		dev_err(dev, "regmap init failed\n");
		// return PTR_ERR(crosslink->regmap);
	}

	v4l2_i2c_subdev_init(&sensor->sd, client, &crosslink_subdev_ops);



	sensor->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	sensor->pad.flags = MEDIA_PAD_FL_SOURCE;
	sensor->sd.entity.ops = &crosslink_sd_media_ops;
	sensor->sd.entity.function = MEDIA_ENT_F_CAM_SENSOR;
	ret = media_entity_pads_init(&sensor->sd.entity, 1, &sensor->pad);
	if (ret)
		return ret;

	mutex_init(&sensor->lock);

	ret = crosslink_init_controls(sensor);
	if (ret)
		goto entity_cleanup;

	ret = v4l2_async_register_subdev_sensor(&sensor->sd);
	if (ret)
		goto free_ctrls;

	pr_debug("<--%s crosslink Probe end successful, return\n",__func__);
	return 0;

free_ctrls:
	pr_debug("---%s crosslink ERR free_ctrls\n",__func__);
	v4l2_ctrl_handler_free(&sensor->ctrls.handler);
entity_cleanup:
	pr_debug("---%s crosslink ERR entity_cleanup\n",__func__);
	media_entity_cleanup(&sensor->sd.entity);
	mutex_destroy(&sensor->lock);
	return ret;
}

static int crosslink_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct crosslink_dev *sensor = to_crosslink_dev(sd);

	v4l2_async_unregister_subdev(&sensor->sd);
	media_entity_cleanup(&sensor->sd.entity);
	v4l2_ctrl_handler_free(&sensor->ctrls.handler);
	mutex_destroy(&sensor->lock);

	return 0;
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
