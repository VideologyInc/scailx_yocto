# devicetree recipe for Karo boards
#
# This bbclass extends OE's devicetree.bbclass by implementing devicetree
# overlays compilation for Videology's BSPs.

SUMMARY = "Videology BSP device-tree overlays"

SRC_URI = " \
	file://dts/freescale/imx8mp-karo.dtsi \
	file://dts/freescale/imx8mp-qsxp-ml81.dts \
	file://dts/freescale/scailx_karo.dts \
	file://dts/freescale/scailx_karo_cameras.dts \
	file://dts/freescale/scailx-imx8mp-cam0-crosslink.dts \
	file://dts/freescale/scailx-imx8mp-cam1-crosslink.dts \
	file://dts/freescale/scailx-imx8mp-cam0-os08a20.dts \
	file://dts/freescale/scailx-imx8mp-cam1-os08a20.dts \
	file://dts/freescale/scailx-imx8mp-cam0-ov5640.dts \
	file://dts/freescale/scailx-imx8mp-cam1-ov5640.dts \
	file://dts/freescale/scailx_karo_gpio_a_spi_overlay.dts \
	file://dts/freescale/scailx_karo_gpio_b_spi_overlay.dts \
	file://dts/freescale/scailx-imx8mp-cam0-vid_isp_ar0234.dts \
	file://dts/freescale/scailx-imx8mp-cam1-vid_isp_ar0234.dts \
	file://dts/freescale/scailx-imx8mp-pcie.dts \
	file://dts/freescale/scailx-imx8mp-vpu_v4l2.dts \
	file://cam-overlays \
"

inherit devicetree

S = "${WORKDIR}/dts/freescale"

COMPATIBLE_MACHINE = ".*(mx8).*"

RM_WORK_EXCLUDE += "${PN}"

do_deploy:append() {
	cd ${DEPLOYDIR}
	install -m 0644 ${WORKDIR}/cam-overlays ${DEPLOYDIR}/devicetree
	tar czf ${DEPLOYDIR}/devicetrees.tgz *
}

DEPENDS += "autoconf-archive-native"
