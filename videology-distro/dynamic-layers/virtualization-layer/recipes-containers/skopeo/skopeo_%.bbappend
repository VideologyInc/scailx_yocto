FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += " file://containers.conf "

do_install:append() {
    install -d ${D}/${sysconfdir}/containers
	install ${WORKDIR}/containers.conf ${D}/${sysconfdir}/containers/containers.conf
}
