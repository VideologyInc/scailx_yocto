SUMMARY = "Include bundled kernel-initramfs into image."
DESCRIPTION = "Installs the bundled kernel with initramfs as a package. \
This requires that variables be set in order to build & bundle the initramfs: \
    INITRAMFS_IMAGE_BUNDLE = '1' \
    INITRAMFS_IMAGE = <my-initramfs> \
"

LICENSE = "CLOSED"

do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install() {
    if [ ! -z "${INITRAMFS_IMAGE}" -a x"${INITRAMFS_IMAGE_BUNDLE}" = x1 ]; then
        install -d "${D}/boot"
        install -m 0644 "${DEPLOY_DIR_IMAGE}/${KERNEL_IMAGETYPE}-initramfs-${MACHINE}.bin" ${D}/boot/${KERNEL_IMAGETYPE}-initramfs
    else
        bberror "INITRAMFS_IMAGE_BUNDLE and INITRAMFS_IMAGE must be set in order to add a bundled kernel-initramfs."
        exit 1
    fi
}
do_install[depends] += "virtual/kernel:do_deploy"

FILES:${PN} += "/boot/*"
