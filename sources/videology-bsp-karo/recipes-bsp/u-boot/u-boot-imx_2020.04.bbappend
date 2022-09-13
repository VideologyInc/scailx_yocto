FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:${THISDIR}/${PN}-${PV}/patches:"
SRC_URI_append = " \
	file://karo.bmp;subdir=git/tools/logos \
"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:${THISDIR}/files:"

SRC_URI_append_mx8-camera += "file://mx8-camera_defconfig;subdir=${S}/configs"
SRC_URI_append_mx8-camera += "file://mx8-camera_mfg_defconfig;subdir=${S}/configs"
SRC_URI_append_mx8-camera += "file://mx8-camera_env.txt;subdir=${S}/board/karo/tx8m"
SRC_URI_append_mx8-camera += "file://0001-add-clk-delay-for-rtl8211.patch"

DEPENDS += "xxd-native"

RM_WORK_EXCLUDE += "${PN}"
