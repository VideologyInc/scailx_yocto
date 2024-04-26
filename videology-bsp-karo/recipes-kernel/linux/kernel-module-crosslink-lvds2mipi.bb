DESCRIPTION = "Kernel loadable module for Crosslink LVDS-MIPI converter"

LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

SRC_URI += "file://crosslink-cam.c;subdir=${S}"
SRC_URI += "file://crosslink-i2c.c;subdir=${S}"
SRC_URI += "file://crosslink-tty.c;subdir=${S}"
SRC_URI += "file://crosslink.h;subdir=${S}"
SRC_URI += "file://Makefile;subdir=${S}"
SRC_URI += "file://crosslink_lvds_B1.bit"
SRC_URI += "file://crosslink-serial.py"

inherit module

do_install:append(){
    install -d ${D}${base_bindir}/
    install -m 0755 "${WORKDIR}/crosslink-serial.py" "${D}${base_bindir}/crosslink-serial.py"
    install -d ${D}${nonarch_base_libdir}/firmware/
    install -m 0644 "${WORKDIR}/crosslink_lvds_B1.bit" "${D}${nonarch_base_libdir}/firmware/crosslink_lvds_B1.bit"
}

FILES:${PN} += "${base_bindir}/"
FILES:${PN} += "${nonarch_base_libdir}/firmware/"

# KERNEL_MODULE_AUTOLOAD = "crosslink_lvds2mipi"

RDEPENDS:${PN} += "serial-xfer"