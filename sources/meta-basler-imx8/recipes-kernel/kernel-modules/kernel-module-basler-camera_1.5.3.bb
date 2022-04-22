SUMMARY = "Basler camera sensor driver"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=801f80980d171dd6425610833a22dbe6"

inherit module

# In case of imx8mplus we need a special modprobe.d config.
RDEPENDS_${PN}_append_mx8mp="imx8mp-modprobe-config"

FILESEXTRAPATHS_prepend := "${THISDIR}/basler-camera:"
SRC_URI = "file://basler-camera-driver.c;subdir=${BPN} file://basler-camera-driver.h;subdir=${BPN} file://Makefile;subdir=${BPN} file://COPYING;subdir=${BPN}"
S = "${WORKDIR}/${BPN}"

FILES_${PN}-dev = "${includedir}/linux/basler-camera-driver.h"
do_install_append() {
    install -d ${D}${includedir}/linux
    install -m 644 ${S}/basler-camera-driver.h ${D}${includedir}/linux/basler-camera-driver.h
    rm -R ${D}${includedir}/${PN}/
}

