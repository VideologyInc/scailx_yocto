DESCRIPTION = "Scailx swupdate container image"

inherit scailx-swupdate-image

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
    avahi-services-ssh \
    avahi-services-sftp \
    packagegroup-imx-core-tools \
    packagegroup-imx-security \
    curl \
    ${DOCKER} \
"

DOCKER ?= "podman podman-compose catatonit fuse-overlayfs "
# slirp4netns

