FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

RDEPENDS_${PN} += "swupdate-config"

SRC_URI += "\
	file://index.html \
	file://favicon.png \
	file://logo.svg \
"

do_install:append () {
	install -m 644 ${WORKDIR}/index.html ${D}/www/
	install -m 644 ${WORKDIR}/favicon.png ${D}/www/images/
	install -m 644 ${WORKDIR}/logo.svg ${D}/www/images/
	rm -rf ${D}/www/images/logo.png
}

RM_WORK_EXCLUDE += "${PN}"
