DESCRIPTION = "Kernel loadable module for Crosslink LVDS-MIPI converter"

LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

SRC_URI += "file://crosslink-cam.c;subdir=${S}"
SRC_URI += "file://crosslink-i2c.c;subdir=${S}"
SRC_URI += "file://Makefile;subdir=${S}"
SRC_URI += "file://crosslink_cs1_res.sh"
SRC_URI += "file://crosslink_lvds_A7.bit"
SRC_URI += "file://crosslink-i2c-serial.py"

inherit module

do_install:append(){
    install -d ${D}${base_bindir}/
    install -m 0755 "${WORKDIR}/crosslink_cs1_res.sh" "${D}${base_bindir}/crosslink_cs1_res.sh"
    install -m 0755 "${WORKDIR}/crosslink-i2c-serial.py" "${D}${base_bindir}/crosslink-i2c-serial.py"
    install -d ${D}/lib/firmware
    install -m 0644 "${WORKDIR}/crosslink_lvds_A7.bit" "${D}/lib/firmware/crosslink_lvds_A7.bit"
}

FILES:${PN} += "${base_bindir}/"
FILES:${PN} += "/lib/firmware/"

# KERNEL_MODULE_AUTOLOAD = "crosslink_lvds2mipi"

RDEPENDS:${PN} += "serial-xfer"