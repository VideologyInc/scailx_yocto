inherit core-image
inherit kernel_devel

inherit link_default_dtb
DEFAULT_DTB = "scailx_karo_crosslink1.dtb"

DESCRIPTION = "Scailx Image. includes most AI/gstreamer related packages and demos"
LICENSE = "MIT"

## Select Image Features
IMAGE_FEATURES += " \
    debug-tweaks \
    tools-profile \
    tools-sdk \
    package-management \
    splash \
    nfs-server \
    tools-debug \
    ssh-server-dropbear \
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
    python3-pyserial \
    gst-variable-rtsp-server \
    kernel-module-crosslink-lvds2mipi \
"

IMAGE_FEATURES:remove = "splash ssh-server-dropbear"
IMAGE_FEATURES += "ssh-server-openssh"

CORE_IMAGE_EXTRA_INSTALL:remove = " \
    packagegroup-tools-bluetooth \
    packagegroup-fsl-tools-audio \
    packagegroup-fsl-tools-gpu-external \
"
