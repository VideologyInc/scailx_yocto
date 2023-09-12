LICENSE = "MIT"

inherit core-image
inherit link_default_dtb
inherit extra-dirs
EXTRA_ROOTFS_DIRS = "storage ${nonarch_libdir}/modules"

inherit squashfs-split

IMAGE_FEATURES += " \
"
# overlayfs-etc
# read-only-rootfs

CORE_IMAGE_EXTRA_INSTALL += " \
    libubootenv-bin \
	u-boot-default-env \
	mmc-utils \
    swupdate \
	swupdate-www \
    swupdate-config \
"

IMAGE_FSTYPES = "squashsplit"

OVERLAYFS_QA_SKIP[storage] = "mount-configured"

