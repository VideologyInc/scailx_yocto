# devicetree recipe for Karo boards
#
# This bbclass extends OE's devicetree.bbclass by implementing devicetree
# overlays compilation for Videology's BSPs.

SUMMARY = "Videology BSP device-tree overlays"
LICENSE = "MIT"
SRC_URI = "git://github.com/VideologyInc/scailx-devicetrees.git;protocol=https;branch=main"
SRCREV = "b40d7bbc7f2759b1e07c5841f66eeb0625b331f4"

inherit devicetree

S = "${WORKDIR}/git/src/arm64/scailx"

COMPATIBLE_MACHINE = ".*(mx8).*"

RM_WORK_EXCLUDE += "${PN}"

do_deploy:append() {
	cd ${DEPLOYDIR}
	install -m 0644 ${WORKDIR}/git/cam-overlays ${DEPLOYDIR}/devicetree
	tar czf ${DEPLOYDIR}/devicetrees.tgz *
}

do_install:append(){
	install -d ${D}${sysconfdir}
	install -m 0644 ${WORKDIR}/git/cam-overlays ${D}${sysconfdir}/cam-overlays
	install -d ${D}${exec_prefix}/src/${PN}
	cp -rf ${WORKDIR}/git/* ${D}${exec_prefix}/src/${PN}/
	rm -rf ${D}${exec_prefix}/src/${PN}/include
}
DEPENDS += "autoconf-archive-native"

FILES:${PN}-dbg += "${exec_prefix}/src/${PN}"
FILES:${PN} += "${sysconfdir}/cam-overlays"


