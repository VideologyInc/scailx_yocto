/*
 * FPGA Manager Driver for Lattice CrossLink.
 *
 *  Copyright (c) 2021 Antmicro Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This driver adds support to the FPGA manager for configuring the SRAM of
 * Lattice CrossLink FPGAs through I2C.
 */

#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/stringify.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/slab.h>
#include "crosslink.h"

#define CROSSLINKPLUS_IDCODE	0x43002F01
#define CROSSLINK_IDCODE		0x43002C01
#define CROSSLINK_RESET_RETRY_CNT	2

u8 isc_enable[]          = {0xC6, 0x00, 0x00, 0x00};
u8 isc_erase[]           = {0x0E, 0x00, 0x00, 0x00};
u8 isc_disable[]         = {0x26, 0x00, 0x00, 0x00};

u8 idcode_pub[]          = {0xE0, 0x00, 0x00, 0x00};
u8 read_usercode[]       = {0xC0, 0x00, 0x00, 0x00};
u8 uidcode_pub[]         = {0x19, 0x00, 0x00, 0x00};

u8 lsc_init[]            = {0x46, 0x00, 0x00, 0x00};
u8 lsc_bitstream_burst[] = {0x7A, 0x00, 0x00, 0x00};
u8 lsc_read_status[]     = {0x3C, 0x00, 0x00, 0x00};
u8 lsc_refresh[]         = {0x79, 0x00, 0x00, 0x00};
u8 lsc_check_busy[]      = {0xF0, 0x00, 0x00, 0x00};

u8 activation_msg[] = {0xA4, 0xC6, 0xF4, 0x8A};

#define STATUS_DONE	BIT(16)
#define STATUS_BUSY	BIT(20)
#define STATUS_FAIL	BIT(21)

#define prog_addr 0x40

static int lsc_xfer(struct i2c_client *client, u8 *write_buf, size_t write_len, u8 *read_buf, size_t read_len) {
    struct i2c_msg msg[2];
    int ret, msg_count = 1;

	if (read_len > 0)
		msg_count = 2;

	memset(msg, 0, sizeof(msg));
    msg[0].addr = prog_addr;
    msg[0].buf = write_buf;
    msg[0].len = write_len;
    msg[1].addr = prog_addr;
    msg[1].flags = I2C_M_RD;
    msg[1].buf = read_buf;
    msg[1].len = read_len;

    ret = i2c_transfer(client->adapter, msg, msg_count);
    return ret;
}

static int crosslink_fpga_reset(struct gpio_desc *reset, struct i2c_client *client)
{
	u32 idcode;
	int ret;

	gpiod_set_value_cansleep(reset, 1);
	mdelay(1);

	ret = lsc_xfer(client, activation_msg, ARRAY_SIZE(activation_msg), NULL, 0);
	if (ret < 0) {
		dev_err(&client->dev, "Writing activation code failed! (%d)\n", ret);
		return ret;
	}

	gpiod_set_value_cansleep(reset, 0);

	mdelay(1);

	ret = lsc_xfer(client, isc_enable, ARRAY_SIZE(isc_enable), NULL, 0);
	if (ret < 0) {
		dev_err(&client->dev, "ISC_ENABLE command failed! (%d)\n", ret);
		return ret;
	}

	ret = lsc_xfer(client, idcode_pub, ARRAY_SIZE(idcode_pub), (u8 *) &idcode, sizeof(idcode));
	if (ret < 0) {
		dev_err(&client->dev, "IDCODE command failed! (%d)\n", ret);
		return ret;
	}

	dev_dbg(&client->dev, "IDCODE: 0x%x\n", idcode);

	if (idcode != CROSSLINK_IDCODE && idcode != CROSSLINKPLUS_IDCODE)
		return -ENODEV;

	return 0;
}

int crosslink_fpga_ops_write_init(struct gpio_desc *reset, struct i2c_client *client)
{
	int ret, i;

	for (i = 0; i < CROSSLINK_RESET_RETRY_CNT; i++) {
		ret = crosslink_fpga_reset(reset, client);
		if (ret == 0)
			break;
	}

	if (ret) {
		dev_err(&client->dev, "FPGA reset failed! (%d)\n", ret);
		return ret;
	}

	ret = lsc_xfer(client, isc_enable, ARRAY_SIZE(isc_enable), NULL, 0);
	if (ret < 0) {
		dev_err(&client->dev, "ISC_ENABLE command failed! (%d)\n", ret);
		return ret;
	}

	mdelay(1);

	ret = lsc_xfer(client, isc_erase, ARRAY_SIZE(isc_erase), NULL, 0);
	if (ret < 0) {
		dev_err(&client->dev, "ISC_ERASE command failed! (%d)\n", ret);
		return ret;
	}

	mdelay(50);

	return 0;
}

int crosslink_fpga_ops_write(struct i2c_client *client, const char *buf, size_t count)
{
	u8* msgbuf = kzalloc(count + 4, GFP_KERNEL);
	int msglen = count + 4;
	int maxlen = 8192;
	int msgnum = DIV_ROUND_UP(msglen,maxlen);
	struct i2c_msg *bitstream_msg;
	u32 status;
	int ret, i;

	msgbuf = kzalloc(count + 4, GFP_KERNEL);
	if (!msgbuf)
		return -ENOMEM;

	bitstream_msg = kzalloc(sizeof(struct i2c_msg) * msgnum, GFP_KERNEL);
	if (!bitstream_msg)
		return -ENOMEM;

	ret = lsc_xfer(client, lsc_init, ARRAY_SIZE(lsc_init), NULL, 0);
	if (ret < 0) {
		dev_err(&client->dev, "LSC_INIT command failed! (%d)\n", ret);
		return ret;
	}

	mdelay(100);

	memcpy(msgbuf, lsc_bitstream_burst, 4);
	memcpy(msgbuf + 4, buf, count);

	for(i = 0; i < msgnum; i++) {
		bitstream_msg[i].addr = prog_addr;
		bitstream_msg[i].buf  = msgbuf + (i * maxlen);
		bitstream_msg[i].len  = msglen > maxlen ? maxlen : msglen;
		if (i > 0)
			bitstream_msg[i].flags = I2C_M_NOSTART;

		msglen -= maxlen;
	}

	ret = i2c_transfer(client->adapter, bitstream_msg, msgnum);
	if (ret < 0) {
		dev_err(&client->dev, "BITSTREAM_BURST command failed! (%d).\n", ret);
		// return ret;
	}

	kfree(msgbuf);
	kfree(bitstream_msg);

	ret = lsc_xfer(client, lsc_read_status, ARRAY_SIZE(lsc_read_status), (u8 *) &status, sizeof(status));
	if (ret < 0) {
		dev_err(&client->dev, "LSC_READ_STATUS command failed! (%d)\n", ret);
		return ret;
	}


	dev_dbg(&client->dev, "STATUS: 0x%x\n (done: %s busy: %s, fail: %s)", status,
			status & STATUS_DONE ? "yes" : "no",
			status & STATUS_BUSY ? "yes" : "no",
			status & STATUS_FAIL ? "yes" : "no");

	if (!(status & STATUS_DONE)) {
		dev_err(&client->dev, "Bitstream loading failed!\n");
		return -EIO;
	}

	return 0;
}

int crosslink_fpga_ops_write_complete(struct i2c_client *client)
{
	int ret;

	ret = lsc_xfer(client, isc_disable, ARRAY_SIZE(isc_disable), NULL, 0);
	if (ret < 0) {
		dev_err(&client->dev, "ISC_DISABLE command failed! (%d)\n", ret);
		return ret;
	}

	dev_info(&client->dev, "Crosslink loading successful!\n");
	return 0;
}

