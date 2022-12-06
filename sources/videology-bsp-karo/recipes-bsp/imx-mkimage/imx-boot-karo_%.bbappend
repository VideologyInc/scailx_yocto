FILESEXTRAPATHS:prepend := "${THISDIR}/mkimage:"
SRC_URI:append = " \
		file://imx8qxb0-bugfixes.patch \
		file://make-clean-bugfix.patch \
		file://make-dependencies.patch \
		file://tx8m-support.patch \
		file://cleanup.patch \
		file://no-tee.patch \
"


do_compile:prepend() {
    export dtbs=${UBOOT_DTB_NAME}
}

do_deploy:append() {
    ln -svf imx-boot-karo ${DEPLOYDIR}/imx-boot
}
