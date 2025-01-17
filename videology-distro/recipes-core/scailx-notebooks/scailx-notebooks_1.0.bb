DESCRIPTION = "Jupyter notebook examples for Scailx devices"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

S = "${WORKDIR}"
inherit allarch systemd

JUPYTER_DIR ?= "/opt/jupyter/notebooks"

SRC_URI  = "file://opencv_capture.ipynb"
SRC_URI += "file://jupyter.service"
SRC_URI += "file://setup_jupyter.sh"

RDEPENDS:${PN} = " python3-virtualenv python3-pip bash "

do_install() {
    install -d ${D}/${JUPYTER_DIR}
    install -m 0755 ${S}/*.ipynb ${D}/${JUPYTER_DIR}/

    install -d "${D}${systemd_system_unitdir}"
    install -m 0644 ${S}/jupyter.service ${D}${systemd_system_unitdir}

    install -d ${D}${bindir}
    install -m 0755 ${S}/setup_jupyter.sh ${D}${bindir}/
}

PACKAGES += "${PN}-boot ${PN}-storage"

FILES:${PN} = "${systemd_system_unitdir} ${bindir}"
FILES:${PN} += "${JUPYTER_DIR}"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "jupyter.service"
SYSTEMD_AUTO_ENABLE:${PN}="disable"
