DEFAULT_DTB ??= "scailx_karo.dtb"
ROOTFS_POSTPROCESS_COMMAND:append = " link_default_dtb; "

link_default_dtb () {
    if [ -e ${IMAGE_ROOTFS}/boot/devicetree/${DEFAULT_DTB} ]; then
        ln -sf ${DEFAULT_DTB} ${IMAGE_ROOTFS}/boot/devicetree/default.dtb
	else
  		bbwarn "default DTB file ${IMAGE_ROOTFS}/boot/devicetree/${DEFAULT_DTB} does not exist"
    fi
}

RM_WORK_EXCLUDE += "${PN}"
