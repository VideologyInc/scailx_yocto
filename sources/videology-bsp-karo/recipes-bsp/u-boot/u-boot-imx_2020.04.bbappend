FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:${THISDIR}/files:"

SRC_URI_append_mx8-camera = " file://mx8-camera_defconfig;subdir=${S}/configs"
SRC_URI_append_mx8-camera = " file://mx8-camera_env.txt;subdir=${S}/board/karo/tx8m"
