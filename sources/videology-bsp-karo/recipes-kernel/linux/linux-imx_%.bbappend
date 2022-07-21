FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI_append_mx8-camera = " file://videology_defconfig;subdir=git/arch/arm64/configs"

KBUILD_DEFCONFIG_mx8-camera = "videology_defconfig"
