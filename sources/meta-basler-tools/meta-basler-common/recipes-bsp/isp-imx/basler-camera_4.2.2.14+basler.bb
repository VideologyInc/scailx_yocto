# Copyright 2020 NXP

DESCRIPTION = "i.MX8M Plus ISP camera driver for basler camera"
LICENSE = "Basler"
LIC_FILES_CHKSUM = "file://COPYING;md5=174e9906a93560022d0901bc38b71e60"

SRC_URI = "file://basler-camera_${PV}.tar.gz;subdir=${S} \
	   file://COPYING;subdir=${S} \
          "

do_install() {
    dest_dir=${D}/opt/imx8-isp/bin
    install -d ${D}/${libdir}
    install -d $dest_dir
    cp -r ${S}/opt/imx8-isp/bin/* $dest_dir
    cp -r ${S}/usr/lib/* ${D}/${libdir}
}

SYSTEMD_AUTO_ENABLE = "enable"

RDEPENDS_${PN} = " isp-imx kernel-module-isp-vvcam"
FILES_${PN} = "${libdir} /opt"
INSANE_SKIP_${PN} = "file-rdeps already-stripped"



