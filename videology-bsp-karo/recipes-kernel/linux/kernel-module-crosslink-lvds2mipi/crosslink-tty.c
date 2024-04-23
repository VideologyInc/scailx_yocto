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

static struct tty_driver *crosslink_tty_driver = NULL;
static struct tty_port_operations  crosslink_tty_port_ops = { };

static void crosslink_tty_handler_old(struct work_struct *work) {
	struct crosslink_dev *sensor = container_of(to_delayed_work(work), struct crosslink_dev, tty_work);
	int ret, xferd;
	uint rx_waiting = 0;
	unsigned char cbuf[64];

	ret  = regmap_read(sensor->regmap, CROSSLINK_REG_UART_RX_CNT, &rx_waiting);
	if (ret==0 && rx_waiting > 0) {
		ret = regmap_bulk_read(sensor->regmap, CROSSLINK_REG_SERIAL, cbuf, rx_waiting);
		dev_dbg_ratelimited(sensor->dev, "%s: TTY got %d bytes\n", __func__, rx_waiting);
		xferd = tty_insert_flip_string(&sensor->port, cbuf, rx_waiting);
		if (xferd)
			tty_flip_buffer_push(&sensor->port);
		usleep_range(1000, 2000);
		schedule_delayed_work(&sensor->tty_work, 0);
	} else {
		schedule_delayed_work(&sensor->tty_work, HZ/50);
	}
}

static int baud_sleep_us(struct tty_port *port){
	int baud=0;
	struct tty_struct *tty;
	tty = tty_port_tty_get(port);
	if (tty)
		baud = tty_termios_baud_rate(&tty->termios);
	if (baud > 0)
		return 12*1000000/baud;
	else
		return 12*1000000/9600;
}

static void crosslink_tty_handler(struct work_struct *work) {
	struct crosslink_dev *sensor = container_of(to_delayed_work(work), struct crosslink_dev, tty_work);
	int ret, xferd;
	uint rx_waiting=0, rx_new=0;
	unsigned char data[64];
	int byte_time = baud_sleep_us(&sensor->port);

	ret  = regmap_read(sensor->regmap, CROSSLINK_REG_UART_RX_CNT, &rx_waiting);
	if (ret==0 && rx_waiting > 0) {
		while (rx_new != rx_waiting) {
			rx_new = rx_waiting;
			usleep_range(byte_time, byte_time << 1);
			regmap_read(sensor->regmap, CROSSLINK_REG_UART_RX_CNT, &rx_waiting);
		}
		ret = regmap_bulk_read(sensor->regmap, CROSSLINK_REG_SERIAL, data, rx_waiting);
		xferd = tty_insert_flip_string(&sensor->port, data, rx_waiting);
		if (xferd != rx_waiting)
			dev_dbg_ratelimited(sensor->dev, "tty_insert_flip_string: %d chars not inserted to flip buffer!\n", (int)rx_waiting - xferd);
		if (xferd)
			tty_flip_buffer_push(&sensor->port);
		dev_dbg_ratelimited(sensor->dev, "%s: TTY got %d bytes\n", __func__, rx_waiting);
	}
	schedule_delayed_work(&sensor->tty_work, HZ/50);
}

static int crosslink_tty_open(struct tty_struct *tty, struct file *filp){
	struct crosslink_dev *sensor = container_of(tty->port, struct crosslink_dev, port);
	if (sensor->dev == NULL)
		return -ENODEV;
	dev_dbg(sensor->dev, "%s: start \n", __func__);
	pm_runtime_get_sync(sensor->dev);
	INIT_DELAYED_WORK(&sensor->tty_work, crosslink_tty_handler);
	schedule_delayed_work(&sensor->tty_work, HZ/50);
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

	dev_dbg_ratelimited(sensor->dev, "%s: CROSSLINK_CMD_SERIAL_TX %d \n", __func__, total);
	ret = regmap_bulk_write(sensor->regmap, CROSSLINK_REG_SERIAL, buf, total);
	if (ret)
		return 0;
	else
		return total;
}

static unsigned int crosslink_tty_write_room(struct tty_struct *tty){
	/* report the space in the crosslink buffer */
	return 32;
}

void crosslink_tty_termois(struct tty_struct *tty, const struct ktermios *old){
	struct crosslink_dev *sensor = container_of(tty->port, struct crosslink_dev, port);
	int baud_old, baud_new;

	baud_new = tty_termios_baud_rate(&tty->termios);
	baud_old = tty_termios_baud_rate(old);

	if (baud_new != baud_old){
		dev_dbg(sensor->dev, "baud rate changed from %d to %d\n", baud_old, baud_new);
		baud_old = DIV_ROUND_CLOSEST(24000000, baud_new);
		regmap_raw_write(sensor->regmap, CROSSLINK_REG_UART_PRESCL, &baud_old, 2);
	}
}

static void crosslink_tty_wait_until_sent(struct tty_struct *tty, int timeout){
	struct crosslink_dev *sensor = container_of(tty->port, struct crosslink_dev, port);
	int uart_stat;
	regmap_read_poll_timeout(sensor->regmap, CROSSLINK_REG_UART_STAT, uart_stat, ((uart_stat & 4) == 4), 1000, 40000); // wait for tx-empty bit
}

static void crosslink_tty_hangup(struct tty_struct *tty) {
	tty_port_hangup(tty->port);
}

static const struct tty_operations crosslink_tty_ops = {
	.open			= crosslink_tty_open,
	.close			= crosslink_tty_close,
	.write			= crosslink_tty_write,
	.hangup			= crosslink_tty_hangup,
	.write_room		= crosslink_tty_write_room,
	.set_termios 	= crosslink_tty_termois,
	.wait_until_sent = crosslink_tty_wait_until_sent,
};

int crosslink_tty_probe(struct crosslink_dev *sensor)
{
	int ret;
	struct device *dev;

	dev_dbg(sensor->dev, "%s: start \n", __func__);
	tty_port_init(&sensor->port);
	sensor->port.ops = &crosslink_tty_port_ops;
	sensor->tty_port_dev = tty_port_register_device(&sensor->port, crosslink_tty_driver, sensor->csi_id, NULL);
	if (IS_ERR(dev)) {
		tty_port_tty_hangup(&sensor->port, false);
		// tty_port_destroy(&sensor->port);
		ret = PTR_ERR(dev);
	}
	return ret;
}

void crosslink_tty_remove(struct crosslink_dev *sensor){
	if (sensor->tty_port_dev) {
		dev_info(sensor->dev, "tty is removed\n");
		// tty_port_free_xmit_buf(&sensor->port);
		tty_unregister_device(crosslink_tty_driver, sensor->csi_id);
		tty_port_tty_hangup(&sensor->port, false);
		// tty_port_destroy(&sensor->port);
	}
}

int crosslink_tty_init(void)
{
	int res;

	pr_debug("crosslink_tty_init\n");
	crosslink_tty_driver = tty_alloc_driver(8, TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV);
	if (IS_ERR(crosslink_tty_driver))
		return PTR_ERR(crosslink_tty_driver);

	crosslink_tty_driver->driver_name = "crosslink_tty";
	crosslink_tty_driver->name = "ttyVISCA";
	crosslink_tty_driver->major = UNNAMED_MAJOR;
	crosslink_tty_driver->minor_start = 0;
	crosslink_tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
	crosslink_tty_driver->subtype = SERIAL_TYPE_NORMAL;
	crosslink_tty_driver->init_termios = tty_std_termios;
	crosslink_tty_driver->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
	crosslink_tty_driver->init_termios.c_ispeed = 9600;
	crosslink_tty_driver->init_termios.c_ospeed = 9600;

	tty_set_operations(crosslink_tty_driver, &crosslink_tty_ops);
	res = tty_register_driver(crosslink_tty_driver);
	if (res) {
		pr_err("failed to register crosslink_tty tty driver\n");
		tty_driver_kref_put(crosslink_tty_driver);
		return res;
	}
	return 0;
}

void crosslink_tty_exit(void)
{
	tty_unregister_driver(crosslink_tty_driver);
	tty_driver_kref_put(crosslink_tty_driver);
}

