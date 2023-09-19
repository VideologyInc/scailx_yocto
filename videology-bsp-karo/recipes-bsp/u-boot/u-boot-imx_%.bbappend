FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
FILESEXTRAPATHS:prepend := "${THISDIR}/u-boot-karo-2020.04:"

SRC_URI:append:scailx-imx8mp = " file://scailx-imx8mp_defconfig;subdir=git/configs "
SRC_URI:append:scailx-imx8mp = " file://scailx-imx8mp_mfg_defconfig;subdir=git/configs "
SRC_URI:append:scailx-imx8mp = " file://scailx-imx8mp_env.txt "
# SRC_URI:append:scailx-imx8mp = "file://0001-add-clk-delay-for-rtl8211.patch"
SRC_URI:append:scailx-imx8mp = " file://0001-increase-eqos-ethernet-phy-reset-delay.patch "

SRC_URI:append = " \
	file://dts/imx8mp-karo.dtsi;subdir=git/arch/arm \
	file://dts/imx8mp-qsxp-ml81-qsbase3-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mp-qsxp-ml81-qsbase3.dts;subdir=git/arch/arm \
	file://dts/imx8mp-qsxp-ml81-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mp-qsxp-ml81.dts;subdir=git/arch/arm \
	file://dts/imx8mp-tx8p-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8m-tx8m-u-boot.dtsi;subdir=git/arch/arm \
"
