// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 NXP
 */

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/virtio.h>
#include <linux/workqueue.h>
#include "crosslink.h"

static struct tty_port_operations  crosslink_tty_port_ops = { };

static void crosslink_tty_handler(struct work_struct *work)
{
	struct crosslink_dev *sensor = container_of(to_delayed_work(work), struct crosslink_dev, tty_work);
	int ret, space;
	uint rx_waiting;
	unsigned char *cbuf;

	printk("%s: start \n", __func__);
	mutex_lock(&sensor->lock);
	ret  = regmap_read(sensor->regmap, CROSSLINK_REG_UART_RX_CNT, &rx_waiting);
	mutex_unlock(&sensor->lock);
	dev_dbg_ratelimited(sensor->dev, "%s: CROSSLINK_CMD_SERIAL_CHECK, %d \n", __func__, rx_waiting);

	space = tty_prepare_flip_string(&sensor->port, &cbuf, (size_t)rx_waiting);
	if (space > 0 && ret == 0){
		mutex_lock(&sensor->lock);
		ret = regmap_bulk_read(sensor->regmap, CROSSLINK_REG_SERIAL, cbuf, rx_waiting);
		mutex_unlock(&sensor->lock);
		if (ret == 0)
		tty_flip_buffer_push(&sensor->port);
	}
	schedule_delayed_work(&sensor->tty_work, 50 * HZ);
}

static int crosslink_tty_install(struct tty_driver *driver, struct tty_struct *tty){
	struct crosslink_dev *sensor = driver->driver_state;
	return tty_port_install(&sensor->port, driver, tty);
}

static int crosslink_tty_open(struct tty_struct *tty, struct file *filp){
	struct crosslink_dev *sensor = container_of(tty->port, struct crosslink_dev, port);
	dev_dbg(sensor->dev, "%s: start \n", __func__);
	pm_runtime_get_sync(sensor->dev);
	INIT_DELAYED_WORK(&sensor->tty_work, crosslink_tty_handler);
	schedule_delayed_work(&sensor->tty_work, 50 * HZ);
	return tty_port_open(tty->port, tty, filp);
}

static void crosslink_tty_close(struct tty_struct *tty, struct file *filp){
	struct crosslink_dev *sensor = container_of(tty->port, struct crosslink_dev, port);
	dev_dbg(sensor->dev, "%s: start \n", __func__);
	cancel_delayed_work_sync(&sensor->tty_work);
	pm_runtime_put_autosuspend(sensor->dev);
	return tty_port_close(tty->port, tty, filp);
}

static int crosslink_tty_write(struct tty_struct *tty, const unsigned char *buf, int total){
	struct crosslink_dev *sensor = container_of(tty->port, struct crosslink_dev, port);
	int ret;

	if (buf == NULL) {
		dev_err(sensor->dev, "buf shouldn't be null.\n");
		return -ENOMEM;
	}

	dev_dbg_ratelimited(sensor->dev, "%s: CROSSLINK_CMD_SERIAL_TX\n", __func__);
	mutex_lock(&sensor->lock);
	ret = regmap_bulk_write(sensor->regmap, CROSSLINK_REG_SERIAL, buf, total);
	mutex_unlock(&sensor->lock);
	if (ret)
		return 0;
	else
		return total;
}

static unsigned int crosslink_tty_write_room(struct tty_struct *tty){
	/* report the space in the rpmsg buffer */
	return 32;
}

void crosslink_tty_termois(struct tty_struct *tty, const struct ktermios *old){
	struct crosslink_dev *sensor = container_of(tty->port, struct crosslink_dev, port);
	int baud_old, baud_new;

	baud_new = tty_termios_baud_rate(&tty->termios);
	baud_old = tty_termios_baud_rate(old);

	if (baud_new != baud_old){
		dev_dbg(sensor->dev, "baud rate changed from %d to %d\n", baud_old, baud_new);
		mutex_lock(&sensor->lock);
		regmap_write(sensor->regmap, CROSSLINK_REG_UART_PRESCL, DIV_ROUND_CLOSEST(24000000, baud_new));
		mutex_unlock(&sensor->lock);
	}
}

// static void crosslink_tty_wait_until_sent(struct tty_struct *tty, int timeout){
// 	struct crosslink_dev *sensor = container_of(tty->port, struct crosslink_dev, port);
// 	mutex_lock(&sensor->lock);
// 	ret  = regmap_read(sensor->regmap, CROSSLINK_REG_UART_RX_CNT, &rx_waiting);
// 	mutex_unlock(&sensor->lock);
// }

static const struct tty_operations crosslink_tty_ops = {
	.install		= crosslink_tty_install,
	.open			= crosslink_tty_open,
	.close			= crosslink_tty_close,
	.write			= crosslink_tty_write,
	.write_room		= crosslink_tty_write_room,
	.set_termios 	= crosslink_tty_termois,
	// .wait_until_sent = crosslink_tty_wait_until_sent,
};

int crosslink_tty_probe(struct crosslink_dev *sensor)
{
	int ret;
	struct tty_driver *crosslink_tty_driver;

	dev_dbg(sensor->dev, "%s: start \n", __func__);

	crosslink_tty_driver = tty_alloc_driver(1, TTY_DRIVER_UNNUMBERED_NODE);
	if (IS_ERR(crosslink_tty_driver))
		return PTR_ERR(crosslink_tty_driver);

	// get number from subdev

	crosslink_tty_driver->driver_name = "crosslink_tty";
	crosslink_tty_driver->name = kasprintf(GFP_KERNEL, "ttyVISCA%d", 0);
	crosslink_tty_driver->major = UNNAMED_MAJOR;
	crosslink_tty_driver->minor_start = 0;
	crosslink_tty_driver->type = TTY_DRIVER_TYPE_CONSOLE;
	crosslink_tty_driver->init_termios = tty_std_termios;
	crosslink_tty_driver->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
	crosslink_tty_driver->init_termios.c_ispeed = 9600;
	crosslink_tty_driver->init_termios.c_ospeed = 9600;

	tty_set_operations(crosslink_tty_driver, &crosslink_tty_ops);

	tty_port_init(&sensor->port);
	sensor->port.ops = &crosslink_tty_port_ops;
	crosslink_tty_driver->driver_state = sensor;
	sensor->crosslink_tty_driver = crosslink_tty_driver;

	ret = tty_register_driver(sensor->crosslink_tty_driver);
	if (ret < 0) {
		pr_err("Couldn't install rpmsg tty driver: ret %d\n", ret);
		goto error1;
	} else {
		pr_info("Install rpmsg tty driver!\n");
	}

	/*
	 * send a message to our remote processor, and tell remote
	 * processor about this channel
	 */
	return 0;

error1:
	tty_driver_kref_put(sensor->crosslink_tty_driver);
	tty_port_destroy(&sensor->port);
	sensor->crosslink_tty_driver = NULL;
	kfree(sensor);

	return ret;
}

void crosslink_tty_remove(struct crosslink_dev *sensor){
	dev_info(sensor->dev, "tty is removed\n");

	tty_unregister_driver(sensor->crosslink_tty_driver);
	kfree(sensor->crosslink_tty_driver->name);
	tty_driver_kref_put(sensor->crosslink_tty_driver);
	tty_port_destroy(&sensor->port);
	sensor->crosslink_tty_driver = NULL;
}


