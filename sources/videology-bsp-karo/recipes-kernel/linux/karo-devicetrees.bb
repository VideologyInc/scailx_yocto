# devicetree recipe for Karo boards
#
# This bbclass extends OE's devicetree.bbclass by implementing devicetree
# overlays compilation for Videology's BSPs.

SUMMARY = "Videology BSP device-tree overlays"

SRC_URI_append_mx8 = " \
	file://dts/freescale/imx8mp-karo.dtsi \
	file://dts/freescale/imx8mp-qsxp-ml81-qsbase3-dsi83.dts \
	file://dts/freescale/imx8mp-qsxp-ml81-qsbase3-laird.dts \
	file://dts/freescale/imx8mp-qsxp-ml81-qsbase3-laird.dtsi \
	file://dts/freescale/imx8mp-qsxp-ml81-qsbase3-raspi-display.dts \
	file://dts/freescale/imx8mp-qsxp-ml81-qsbase3-raspi-display.dtsi \
	file://dts/freescale/imx8mp-qsxp-ml81-qsbase3-tc358867.dts \
	file://dts/freescale/imx8mp-qsxp-ml81-qsbase3.dts \
	file://dts/freescale/imx8mp-qsxp-ml81-qsbase3.dtsi \
	file://dts/freescale/imx8mp-qsxp-ml81.dts \
	file://dts/freescale/imx8mp-qsxp-ml81-qsbase3-basler.dts \
"

inherit devicetree

S = "${WORKDIR}/dts/freescale"

COMPATIBLE_MACHINE = ".*(mx8).*"
