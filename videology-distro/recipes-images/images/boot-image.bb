DESCRIPTION = "kernel and devicetree image"
LICENSE = "MIT"

# PACKAGE_INSTALL = " "

IMAGE_FEATURES = ""
IMAGE_INSTALL = " karo-devicetrees kernel-initramfs "

export IMAGE_BASENAME = "boot-image"

IMAGE_FSTYPES = "tar.gz"
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

