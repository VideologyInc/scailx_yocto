# devicetree recipe for Karo boards
#
# This bbclass extends OE's devicetree.bbclass by implementing devicetree
# overlays compilation for Videology's BSPs.

SUMMARY = "Videology BSP device-tree overlays"
LICENSE = "MIT"
SRC_URI = "git://github.com/VideologyInc/scailx-devicetrees.git;protocol=https;branch=main"
SRCREV  = "eb3430c0232c6b70d249b85367c6c924b97248e9"
# SRCREV  = "eabb186d12b7ab2f4a5caf411a1113eddd09d718"

inherit devicetree

S = "${WORKDIR}/git/src/arm64/scailx"

COMPATIBLE_MACHINE = ".*(mx8).*"

RM_WORK_EXCLUDE += "${PN}"

pkg_postinst:${PN} () {
    echo "0x40 lvds2mipi.dtbo"      >> $D/boot/devicetree/cam-overlays
    echo "0x38 vid_isp_ar0234.dtbo" >> $D/boot/devicetree/cam-overlays
    echo "0x36 os08a20.dtbo"        >> $D/boot/devicetree/cam-overlays
    echo "0x57 sony-mipi.dtbo"      >> $D/boot/devicetree/cam-overlays
}

do_install:append(){
	install -d ${D}${exec_prefix}/src/${PN}
	cp -rf ${WORKDIR}/git/* ${D}${exec_prefix}/src/${PN}/
	rm -rf ${D}${exec_prefix}/src/${PN}/include
}

FILES:${PN}-dbg += "${exec_prefix}/src/${PN}"
FILES:${PN} += "${sysconfdir}/cam-overlays"


