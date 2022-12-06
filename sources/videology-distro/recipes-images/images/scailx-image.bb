#based on imx-image-full, but without QT and a few other packages.
require recipes-fsl/images/imx-image-multimedia.bb

inherit link_default_dtb

DEFAULT_DTB = "imx8mp-qsxp-ml81-camera-OV5640.dtb"

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
