DESCRIPTION = "Scailx swupdate container image"

inherit scailx-swupdate-image

DEFAULT_DTB = "scailx_karo_cameras.dtb"

IMAGE_FEATURES += " \
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
    p11-kit \
    opensc openssl-bin libp11 \
    packagegroup-imx-core-tools \
    packagegroup-imx-security \
    curl \
    ${DOCKER} \
"

#Drivers
IMAGE_INSTALL += " kernel-module-crosslink-lvds2mipi "

DOCKER ?= "docker docker-compose fuse-overlayfs "
# slirp4netns

