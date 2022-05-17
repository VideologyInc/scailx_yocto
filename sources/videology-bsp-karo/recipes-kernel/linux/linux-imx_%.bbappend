FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI_append = " file://videology_defconfig;subdir=git/arch/arm64/configs"

KBUILD_DEFCONFIG_mx8mp = "videology_defconfig"
