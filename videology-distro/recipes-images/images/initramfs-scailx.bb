DESCRIPTION = "Scailx initramfs image"
LICENSE = "MIT"

VIRTUAL-RUNTIME_dev_manager ?= "busybox-mdev"

PACKAGE_INSTALL = " \
	initramfs-framework-base \
	initramfs-module-debug \
	initramfs-module-mdev \
	initramfs-module-rootfs \
	busybox \
	${VIRTUAL-RUNTIME_base-utils} \
	base-passwd e2fsprogs-e2fsck \
	i2c-tools \
	${ROOTFS_BOOTSTRAP_INSTALL} \
	"

# PACKAGE_INSTALL += " initramfs-module-overlayroot "
PACKAGE_INSTALL += " initramfs-module-swupdate "
PACKAGE_INSTALL += " initramfs-module-storage "

# PACKAGE_INSTALL += " initramfs-module-nfsrootfs "

PACKAGE_INSTALL:append:scailx-swu = " \
		base-passwd \
		busybox \
		mtd-utils \
		libconfig \
		mmc-utils \
		libubootenv-bin \
		u-boot-default-env \
		swupdate \
		swupdate-config \
		swupdate-www \
        initscripts \
		util-linux-sfdisk \
	"

# SYSTEMD_DEFAULT_TARGET = "initrd.target"

# Do not pollute the initrd image with rootfs features
IMAGE_FEATURES = ""

export IMAGE_BASENAME = "initramfs-scailx"
IMAGE_LINGUAS = ""

IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"
IMAGE_FSTYPES:remove = "wic wic.gz wic.bmap wic.nopt ext4 ext4.gz"

# avoid circular dependencies
EXTRA_IMAGEDEPENDS = ""

inherit core-image nopackages

IMAGE_ROOTFS_SIZE = "8192"

# Users will often ask for extra space in their rootfs by setting this
# globally.  Since this is a initramfs, we don't want to make it bigger
IMAGE_ROOTFS_EXTRA_SPACE = "0"
IMAGE_OVERHEAD_FACTOR = "1.0"

RM_WORK_EXCLUDE += "${PN}"

