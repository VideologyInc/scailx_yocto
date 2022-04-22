# Copyright 2020 Basler

DESCRIPTION = "i.MX8M Plus ISP stack"
LICENSE = "Basler & MIT"
LIC_FILES_CHKSUM = "file://COPYING;md5=8fa3130fd6cc076743b929f50b7be06a"

SRC_URI = "file://init\
           file://isp-imx_${PV}.tar.gz;subdir=${S}\
	         file://COPYING;subdir=${S}\
          "

inherit update-rc.d systemd

do_install() {
    install -Dm755 ${WORKDIR}/init ${D}/${sysconfdir}/init.d/isp-imx

    install -d ${D}/${libdir}
    install -d ${D}/opt/imx8-isp/bin
    cp -r ${S}/opt/imx8-isp/bin/* ${D}/opt/imx8-isp/bin
    cp -r ${S}/usr/lib/* ${D}/${libdir}
    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
      install -d ${D}${systemd_system_unitdir}
      install -d ${D}/etc/default
      install -m 0644 ${S}/lib/systemd/system/imx8-isp.service ${D}${systemd_system_unitdir}
      install -m 0644 ${S}/etc/default/imx8-isp ${D}/etc/default
     fi
}

RDEPENDS_${PN} = "libdrm bash procps imx-gpu-g2d kernel-module-isp-vvcam"
SYSTEMD_AUTO_ENABLE = "enable"
SYSTEMD_SERVICE_${PN} = "imx8-isp.service"
PACKAGES = "${PN}"

FILES_${PN} = "${libdir} /opt ${systemd_system_unitdir}/imx8-isp.service /etc/default ${sysconfdir}/init.d/isp-imx"

INSANE_SKIP_${PN} += "rpaths dev-deps dev-so already-stripped"

INITSCRIPT_NAME = "isp-imx"
