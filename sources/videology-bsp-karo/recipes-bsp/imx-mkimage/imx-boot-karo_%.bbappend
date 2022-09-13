FILESEXTRAPATHS_prepend := "${THISDIR}/mkimage:"
SRC_URI_append = " \
		file://imx8qxb0-bugfixes.patch \
		file://make-clean-bugfix.patch \
		file://make-dependencies.patch \
		file://tx8m-support.patch \
		file://cleanup.patch \
		file://no-tee.patch \
"


do_compile_prepend() {
    export dtbs=${UBOOT_DTB_NAME}
}

do_deploy_append() {
    ln -svf imx-boot-karo ${DEPLOYDIR}/imx-boot
}
