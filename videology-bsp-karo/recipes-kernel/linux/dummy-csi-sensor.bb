DESCRIPTION = "Kernel loadable module for csi-sensor device without controls or I2C. Usefull for FPGA or similar."
HOMEPAGE = "https://github.com/6by9/linux/blob/rpi-5.15.y-dummy_sensor/drivers/media/i2c/dummy_sensor.c"

LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

SRC_URI = "file://dummy_csi_sensor.c;subdir=${S}"
SRC_URI += "file://Makefile;subdir=${S}"

inherit module

RDEPENDS:${PN} += "serial-xfer"

RM_WORK_EXCLUDE += "${PN}"
