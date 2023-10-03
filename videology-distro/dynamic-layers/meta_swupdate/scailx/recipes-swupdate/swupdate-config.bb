FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
DESCRIPTION = "Device-specific swupdate config files"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"


SRC_URI += "\
	file://swupdate.cfg \
	file://swupdate.default \
    file://swupdate-env \
"

do_compile () {
    echo "SWUPDATE_ARGS='-v -H ${MACHINE}:1.0'" > ${WORKDIR}/hardware
}

do_install:append () {
	install -d ${D}${sysconfdir}/default/
    install -d ${D}${sysconfdir}/swupdate/conf.d/
    install -d ${D}/usr/bin/

    install -m 644 ${WORKDIR}/swupdate.cfg ${D}${sysconfdir}/
	install -m 644 ${WORKDIR}/swupdate.default ${D}${sysconfdir}/default/swupdate
    install -m 644 ${WORKDIR}/hardware ${D}${sysconfdir}/swupdate/conf.d/20-hardware
    install -m 755 ${WORKDIR}/swupdate-env ${D}/usr/bin/swupdate-env
}

FILES_${PN} += "${sysconfdir} /usr/bin/swupdate-env"

RM_WORK_EXCLUDE += "${PN}"
