DESCRIPTION = "Modules only squashfs image"
LICENSE = "MIT"

PACKAGE_INSTALL = " kernel-modules kernel "

IMAGE_FEATURES = ""
IMAGE_INSTALL = ""

export IMAGE_BASENAME = "modules-image"

IMAGE_FSTYPES = "squashfs"
IMAGE_CLASSES := ""

# avoid circular dependencies
EXTRA_IMAGEDEPENDS = ""

inherit core-image nopackages

IMAGE_ROOTFS_SIZE = "8192"

# Users will often ask for extra space in their rootfs by setting this
# globally.  Since this is a initramfs, we don't want to make it bigger
IMAGE_ROOTFS_EXTRA_SPACE = "0"
IMAGE_OVERHEAD_FACTOR = "1.0"

RM_WORK_EXCLUDE += "${PN}"

