# imx-isp custom service for scailx
#

SUMMARY = "i.MX 8Mplus ISP daemon replacement for scailx"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI += "file://${BPN}.service"
SRC_URI += "file://start_isp_scailx.sh"

SRC_URI += "file://scailx_sensors/"
# file://scailx_sensors/basler.sh \
# file://scailx_sensors/daa3840.sh
# file://scailx_sensors/daa.sh
# file://scailx_sensors/imx662.sh
# file://scailx_sensors/imx676.sh
# file://scailx_sensors/imx678.sh
# file://scailx_sensors/imx900.sh
# file://scailx_sensors/os08a20.sh

COMPATIBLE_MACHINE = ".*(mx8).*"

RM_WORK_EXCLUDE += "${PN}"

inherit systemd

do_install(){
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/${BPN}.service ${D}${systemd_system_unitdir}/${BPN}.service

    install -d ${D}/opt/imx8-isp/bin/scailx_sensors
    install -m 0755 ${WORKDIR}/start_isp_scailx.sh ${D}/opt/imx8-isp/bin/start_isp_scailx.sh
    for f in ${WORKDIR}/scailx_sensors/*; do install -m 0755 $f ${D}/opt/imx8-isp/bin/scailx_sensors/ ; done
}

SYSTEMD_SERVICE:${PN} = "${BPN}.service"

RDEPENDS:${PN} += "isp-imx bash"
FILES:${PN} += "/opt/imx8-isp/bin/"