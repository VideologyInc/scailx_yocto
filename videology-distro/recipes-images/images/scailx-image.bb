inherit core-image
inherit kernel_devel

inherit link_default_dtb
DEFAULT_DTB = "scailx_karo_cameras.dtb"

DESCRIPTION = "Scailx Image. includes most AI/gstreamer related packages and demos"
LICENSE = "MIT"

IMAGE_FSTYPES += 'uuuimg wic'

## Select Image Features
IMAGE_FEATURES += " \
    debug-tweaks \
    tools-profile \
    tools-sdk \
    package-management \
    splash \
    nfs-server \
    tools-debug \
    ssh-server-openssh \
    tools-testapps \
    hwcodecs \
    ${@bb.utils.contains('DISTRO_FEATURES', 'wayland', 'weston', \
       bb.utils.contains('DISTRO_FEATURES',     'x11', 'x11-base x11-sato', \
                                                       '', d), d)} \
"

DOCKER ?= "podman podman-compose"

G2D_SAMPLES              = ""
G2D_SAMPLES:imxgpu2d     = "imx-g2d-samples"
G2D_SAMPLES:mx93-nxp-bsp = "imx-g2d-samples"

CORE_IMAGE_EXTRA_INSTALL += " \
    packagegroup-core-full-cmdline \
    packagegroup-fsl-tools-testapps \
    packagegroup-imx-isp \
    packagegroup-imx-security \
    packagegroup-fsl-gstreamer1.0 \
    packagegroup-fsl-gstreamer1.0-full \
    gstreamer1.0-python \
    python3-pycairo \
    git \
    firmwared \
    ${@bb.utils.contains('DISTRO_FEATURES', 'x11 wayland', 'weston-xwayland xterm', '', d)} \
    ${DOCKER} \
    ${G2D_SAMPLES} \
"

CONFLICT_DISTRO_FEATURES = "directfb"

# Add opencv for i.MX GPU
OPENCV_PKGS       ?= ""
OPENCV_PKGS:imxgpu = " \
    opencv-apps \
    opencv-samples \
    python3-opencv \
"

IMAGE_INSTALL += " \
    packagegroup-imx-ml \
    sclbl \
    tzdata \
    ${OPENCV_PKGS} \
    openssh-sftp-server \
    xauth \
    gnupg \
    dtc \
    i2c-tools \
    gst-perf \
    libubootenv \
    fuse-exfat \
    v4l2loopback \
    v4l2loopback-utils \
    python3-pyserial \
    gst-variable-rtsp-server \
    kernel-module-crosslink-lvds2mipi \
"

CORE_IMAGE_EXTRA_INSTALL:remove = " \
    packagegroup-tools-bluetooth \
    packagegroup-fsl-tools-audio \
    packagegroup-fsl-tools-gpu-external \
"

TARGET_HOSTNAME ?= "scailx-zb"

ROOTFS_POSTPROCESS_COMMAND:append = " hostname_change; "
hostname_change () {
    echo "${TARGET_HOSTNAME}" > ${IMAGE_ROOTFS}${sysconfdir}/hostname
}