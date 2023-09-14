DESCRIPTION = "Dummy image used to run wik sdcard image creation. "
LICENSE = "MIT"

inherit core-image
WKS_FILE = "boot-storage.wks.in"

do_rootfs[depends] += "scailx-image-swupdate:do_image_complete"

IMAGE_FEATURES += " \
    debug-tweaks \
    tools-profile \
    tools-sdk \
    tools-debug \
    ssh-server-openssh \
    hwcodecs \
"

IMAGE_INSTALL += " \
    ${@bb.utils.contains('DISTRO_FEATURES', 'x11 wayland', 'weston-xwayland xterm', '', d)} \
    imx-test \
    firmwared \
	openssh-sftp-server \
	avahi-services-ssh \
    avahi-services-sftp \
    packagegroup-imx-core-tools \
    packagegroup-imx-security \
    curl \
    ${DOCKER} \
"

DOCKER            ?= "podman podman-compose"

export IMAGE_BASENAME = "scailx-swupdate-image"

CORE_IMAGE_EXTRA_INSTALL += " \
	swupdate \
	swupdate-www \
"

IMAGE_FSTYPES = 'uuuimg wic'

