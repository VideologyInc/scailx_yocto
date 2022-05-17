SUMMARY = "A very basic Wayland image with Weston desktop and terminal"

inherit features_check
require recipes-graphics/images/core-image-weston.bb
require karo-image.inc

IMAGE_FEATURES_remove = "ssh-server-dropbear"
IMAGE_FEATURES += "ssh-server-openssh"

LICENSE = "MIT"

REQUIRED_DISTRO_FEATURES = "wayland"

CORE_IMAGE_BASE_INSTALL_append = " \
        glmark2 \
        weston \
        weston-init \
        weston-examples \
"

IMAGE_INSTALL_append = " \
        gst-examples \
        libdrm \
        libdrm-tests \
        libdrm-kms \
        ${@bb.utils.contains('DISTRO_FEATURES', 'x11 wayland', 'weston-xwayland xterm', '', d)} \
"

IMAGE_INSTALL_append_use-mainline-bsp = " \
        libdrm-etnaviv \
"

# karo-image-weston won't fit in any of our nand modules!
IMAGE_FSTYPES_remove = "ubi"

ROOTFS_PARTITION_SIZE = "2097152"

QB_MEM = '${@bb.utils.contains("DISTRO_FEATURES", "opengl", "-m 512", "-m 256", d)}'
