FILESEXTRAPATHS:prepend := "${THISDIR}/busybox:"

SRC_URI += " file://bootchart.cfg "
SRC_URI += " file://luksid.cfg "

SRC_URI += " file://zcip.cfg "
SRC_URI += " file://zcip.script "
do_install:append() {
    install -d ${D}${sysconfdir}/
    install -m 0755 ${WORKDIR}/zcip.script ${D}${sysconfdir}/zcip.script
}
FILES:${PN} += "${sysconfdir}/zcip.script"

