FILESEXTRAPATHS:prepend := "${THISDIR}/${BP}:${THISDIR}/${BP}/patches:"
SRC_URI:append = " \
	file://karo.bmp;subdir=git/tools/logos \
"

SRC_URI:append = " file://0001-increase-eqos-ethernet-phy-reset-delay.patch "
SRC_URI += "file://fw_env.config"
# SRC_URI:append = " file://0001-add-clk-delay-for-rtl8211.patch "
UBOOT_ENV_FILE ?= "${MACHINE}_env.txt"


SRC_URI:append = " \
	${@ "file://${UBOOT_ENV_FILE};subdir=git/board/karo/tx8m" if "${UBOOT_ENV_FILE}" != "" else ""} \
	file://dts/imx8m-tx8m-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mm-karo.dtsi;subdir=git/arch/arm \
	file://dts/imx8mm-qsxm-mm60-qsbase3-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mm-qsxm-mm60-qsbase3.dts;subdir=git/arch/arm \
	file://dts/imx8mm-qsxm-mm60-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mm-qsxm-mm60.dts;subdir=git/arch/arm \
	file://dts/imx8mm-tx8m-1610-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mm-tx8m-1610.dts;subdir=git/arch/arm \
	file://dts/imx8mm-tx8m-1620-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mm-tx8m-1620.dts;subdir=git/arch/arm \
	file://dts/imx8mm-tx8m-1622-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mm-tx8m-1622.dts;subdir=git/arch/arm \
	file://dts/imx8mm-tx8m-16xx.dtsi;subdir=git/arch/arm \
	file://dts/imx8mm-tx8m-mipi-mb.dtsi;subdir=git/arch/arm \
	file://dts/imx8mm-tx8mm-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mp-karo.dtsi;subdir=git/arch/arm \
	file://dts/imx8mp-qsxp-ml81-qsbase4-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mp-qsxp-ml81-qsbase4.dts;subdir=git/arch/arm \
	file://dts/imx8mp-qsxp-ml81-qsbase3-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mp-qsxp-ml81-qsbase3.dts;subdir=git/arch/arm \
	file://dts/imx8mp-qsxp-ml81-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mp-qsxp-ml81.dts;subdir=git/arch/arm \
	file://dts/imx8mp-tx8p-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mp-tx8p-ml81-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mp-tx8p-ml81.dts;subdir=git/arch/arm \
	file://dts/imx8mp-tx8p-ml82-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mp-tx8p-ml82.dts;subdir=git/arch/arm \
"

UBOOT_BOARD_DIR = "board/karo/tx8m"
UBOOT_FEATURES:append = "${@ bb.utils.contains('DISTRO_FEATURES', "copro", " copro", "", d)}"
UBOOT_FEATURES:append = " fastboot"

do_fetch:prepend () {
    if d.getVar('KARO_BASEBOARD') != "" and d.getVar('KARO_BASEBOARD') not in d.getVar('KARO_BASEBOARDS').split():
        raise_sanity_error("Module %s is not supported on Baseboard '%s'; available baseboards are:\n%s" % \
            (d.getVar('MACHINE'), d.getVar('KARO_BASEBOARD'), "\n".join(d.getVar('KARO_BASEBOARDS').split())), d)
}

