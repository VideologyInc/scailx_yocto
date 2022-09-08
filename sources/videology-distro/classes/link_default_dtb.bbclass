DEFAULT_DTB ??= "imx8mp-qsxp-ml81-qsbase3-basler.dtb"
ROOTFS_POSTPROCESS_COMMAND_append = " link_default_dtb; "

link_default_dtb () {
    if [ -e ${IMAGE_ROOTFS}/boot/devicetree/${DEFAULT_DTB} ]; then
        ln -sf ${DEFAULT_DTB} ${IMAGE_ROOTFS}/boot/devicetree/default.dtb
	else
  		bbwarn "default DTB file ${IMAGE_ROOTFS}/boot/devicetree/${DEFAULT_DTB} does not exist"
    fi
}
