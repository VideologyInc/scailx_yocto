DESCRIPTION = "Kernel loadable module for Crosslink LVDS-MIPI converter"

LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

SRC_URI = "git://github.com/VideologyInc/kernel-module-crosslink.git;branch=master;protocol=https"
SRCREV = "52df9f1bd9af242d0015b7e4bb88a21ab3f4228b"

S = "${WORKDIR}/git"

inherit module

do_install:append(){
    install -d ${D}${base_bindir}/
    install -m 0755 "${S}/crosslink-serial.py" "${D}${base_bindir}/crosslink-serial.py"
    install -d ${D}${nonarch_base_libdir}/firmware/
    install -m 0644 "${S}/crosslink_lvds_B1.bit" "${D}${nonarch_base_libdir}/firmware/crosslink_lvds_B1.bit"
}

FILES:${PN} += "${base_bindir}/"
FILES:${PN} += "${nonarch_base_libdir}/firmware/"

# KERNEL_MODULE_AUTOLOAD = "crosslink_lvds2mipi"

RDEPENDS:${PN} += "serial-xfer"