FILESEXTRAPATHS_prepend := "${THISDIR}/linux-imx:"

require linux-imx_5.10.inc

SRC_URI_append = " \
        file://dts/freescale/imx8mp-qsxp-ml81-qsbase3-basler.dts;subdir=git/arch/arm64/boot \
        file://dts/freescale/imx8mm-qsxm-mm60-qsbase3-basler.dts;subdir=git/arch/arm64/boot \
"
