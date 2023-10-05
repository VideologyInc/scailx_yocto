DESCRIPTION = "initramfs- only image to uuu onto production devices. Fetches full install from inline."
LICENSE = "MIT"

# PACKAGE_INSTALL = " "

IMAGE_FEATURES = ""
IMAGE_INSTALL = " karo-devicetrees "

DEPENDS += "scailx-boot-script "
export IMAGE_BASENAME = "scailx-bootstrap-image"

IMAGE_FSTYPES = "uuuimg wic"

# avoid circular dependencies
EXTRA_IMAGEDEPENDS = ""

inherit core-image nopackages

IMAGE_ROOTFS_SIZE = "8192"

# Users will often ask for extra space in their rootfs by setting this
# globally.  Since this is a initramfs, we don't want to make it bigger
IMAGE_ROOTFS_EXTRA_SPACE = "0"
IMAGE_OVERHEAD_FACTOR = "1.0"

RM_WORK_EXCLUDE += "${PN}"

OVERLAYFS_QA_SKIP[storage] = "mount-configured"