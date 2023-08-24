DESCRIPTION = "Scailx swupdate image"
LICENSE = "MIT"

inherit core-image
inherit link_default_dtb
inherit extra-dirs
EXTRA_ROOTFS_DIRS = "storage containers config"

DEFAULT_DTB = "scailx_karo_crosslink1.dtb"

IMAGE_FEATURES += " \
    debug-tweaks \
    tools-profile \
    ssh-server-openssh \
    hwcodecs \
"
# read-only-rootfs
# tools-sdk
# tools-debug

IMAGE_INSTALL += " \
    ${@bb.utils.contains('DISTRO_FEATURES', 'x11 wayland', 'weston-xwayland xterm', '', d)} \
    firmwared \
	openssh-sftp-server \
	xauth \
    gnupg \
    dtc \
    i2c-tools \
    libubootenv-bin \
	u-boot-default-env \
	avahi-services-ssh \
    avahi-services-sftp \
    packagegroup-imx-core-tools \
    packagegroup-imx-security \
    curl \
    ${DOCKER} \
"

DOCKER ?= "podman podman-compose catatonit fuse-overlayfs "
# slirp4netns

CORE_IMAGE_EXTRA_INSTALL += " \
	mmc-utils \
    swupdate \
	swupdate-www \
    swupdate-config \
"

IMAGE_FSTYPES = "squashfs"
