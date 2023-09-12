DESCRIPTION = "Scailx swupdate container image"

inherit scailx-swupdate-image

DEFAULT_DTB = "scailx_karo_crosslink1.dtb"

IMAGE_FEATURES += " \
    debug-tweaks \
    tools-profile \
    ssh-server-openssh \
    hwcodecs \
    ${@bb.utils.contains('DISTRO_FEATURES', 'wayland', 'weston', bb.utils.contains('DISTRO_FEATURES', 'x11', 'x11-base x11-sato', '', d), d)} \
"
# read-only-rootfs
# tools-sdk
# tools-debug

CONFLICT_DISTRO_FEATURES = "directfb"

# Add opencv for i.MX GPU
OPENCV_PKGS       ?= ""
OPENCV_PKGS:imxgpu = " \
    opencv-apps \
    opencv-samples \
    python3-opencv \
"

DOCKER ?= "podman podman-compose catatonit fuse-overlayfs podman-tui"
# slirp4netns

IMAGE_INSTALL += " \
    ${OPENCV_PKGS} \
    packagegroup-ml-demoless \
	openssh-sftp-server \
    gst-perf \
    fuse-exfat \
    kernel-modules \
	xauth \
    gnupg \
    dtc \
    lsof \
    i2c-tools \
    v4l-utils \
    v4l2loopback \
    v4l2loopback-utils \
    avahi-services-ssh \
    avahi-services-sftp \
    python3-pyserial \
    gst-variable-rtsp-server \
    kernel-module-crosslink-lvds2mipi \
    packagegroup-imx-core-tools \
    packagegroup-imx-security \
    curl \
    git \
    python3-pycairo \
    ${DOCKER} \
"

CORE_IMAGE_EXTRA_INSTALL += " \
    packagegroup-core-full-cmdline \
    packagegroup-self-hosted-sdk \
    packagegroup-imx-isp \
    packagegroup-imx-security \
    packagegroup-fsl-gstreamer1.0 \
    packagegroup-fsl-gstreamer1.0-full \
    packagegroup-fsl-gstreamer1.0-commercial \
    gstreamer1.0-python \
    firmwared \
    ${@bb.utils.contains('DISTRO_FEATURES', 'x11 wayland', 'weston-xwayland xterm', '', d)} \
"

TARGET_HOSTNAME ?= "scailx-zb"

ROOTFS_POSTPROCESS_COMMAND:append = " hostname_change; "
hostname_change () {
    echo "${TARGET_HOSTNAME}" > ${IMAGE_ROOTFS}${sysconfdir}/hostname
}

ROOTFS_POSTPROCESS_COMMAND:append = " cleanup_dirs; "
cleanup_dirs () {
    rm -rf ${IMAGE_ROOTFS}/usr/lib/python3.10/site-packages/torch/test/
}

