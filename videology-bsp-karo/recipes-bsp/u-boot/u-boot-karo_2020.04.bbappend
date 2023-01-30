FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
FILESEXTRAPATHS:prepend := "${THISDIR}/${BP}:${THISDIR}/${BP}/patches:${THISDIR}/${BP}/env:"

SRC_URI:append:mx8-camera = " file://mx8-camera_defconfig;subdir=git/configs "
SRC_URI:append:mx8-camera = " file://mx8-camera_mfg_defconfig;subdir=git/configs "
SRC_URI:append:mx8-camera = " file://0001-add-clk-delay-for-rtl8211.patch "
SRC_URI:append:mx8-camera = " file://0001-increase-eqos-ethernet-phy-reset-delay.patch "

UBOOT_ENV_FILE ?= "${MACHINE}_env.txt"


SRC_URI:append = " \
	${@ "file://${UBOOT_ENV_FILE};subdir=git/board/karo/tx8m" if "${UBOOT_ENV_FILE}" != "" else ""} \
	file://dts/imx8mp-karo.dtsi;subdir=git/arch/arm \
	file://dts/imx8mp-qsxp-ml81-qsbase3-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mp-qsxp-ml81-qsbase3.dts;subdir=git/arch/arm \
	file://dts/imx8mp-qsxp-ml81-u-boot.dtsi;subdir=git/arch/arm \
	file://dts/imx8mp-qsxp-ml81.dts;subdir=git/arch/arm \
"

DEPENDS += "xxd-native"
