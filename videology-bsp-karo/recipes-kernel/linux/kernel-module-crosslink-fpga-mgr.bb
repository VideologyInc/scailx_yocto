DESCRIPTION = "FPGA bitfile loading via I2C for crosslink devices. \
               Copyright (c) 2021 Antmicro Ltd"
HOMEPAGE = "https://github.com/antmicro/sdi-mipi-bridge-linux-rpi"

LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

SRC_URI += "file://crosslink-i2c.c;subdir=${S}"
SRC_URI += "file://Makefile;subdir=${S}"

inherit module

KERNEL_MODULE_AUTOLOAD = "crosslink-i2c"
