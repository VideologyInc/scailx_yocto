DESCRIPTION = "Scailx swupdate container image"

inherit scailx-swupdate-image

DEFAULT_DTB = "scailx_karo.dtb"

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
    avahi-services-ssh \
    avahi-services-sftp \
    packagegroup-imx-core-tools \
    packagegroup-imx-security \
    curl \
    ${DOCKER} \
"

#Drivers
IMAGE_INSTALL += " kernel-module-crosslink-lvds2mipi "

DOCKER ?= "docker-ce python3-docker-compose fuse-overlayfs "
# slirp4netns

