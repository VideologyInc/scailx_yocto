#
# Based on examples from Scott Murray (Building Container Images with
# OpenEmbedded and the Yocto Project) ELCe 2018
#
SUMMARY = "Basic container image"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

IMAGE_FSTYPES = "container oci"
inherit image
inherit core-image
inherit image-oci

inherit kernel_devel

IMAGE_FEATURES = " \
    debug-tweaks \
    tools-profile \
    tools-sdk \
    ssh-server-openssh \
    tools-debug \
"
IMAGE_LINGUAS = ""
NO_RECOMMENDATIONS = "1"

IMAGE_INSTALL = " \
        base-files \
        base-passwd \
        netbase \
        openssh-sftp-server \
        bash \
        busybox \
"

IMAGE_INSTALL += " \
    packagegroup-fsl-gstreamer1.0 \
    packagegroup-imx-ml \
    packagegroup-fsl-opencv-imx \
"
IMAGE_INSTALL += "packagegroup-self-hosted-sdk"

# Add opencv for i.MX GPU
OPENCV_PKGS       ?= ""
OPENCV_PKGS:imxgpu = " \
    opencv-apps \
    python3-opencv \
"

IMAGE_INSTALL += " \
    pyvidctrl \
    ${OPENCV_PKGS} \
    gst-perf \
    dtc \
    lsof \
    meson \
    ninja \
    nodejs \
    nodejs-npm \
    go \
    i2c-tools \
    pyhailort \
    hailortcli \
    amazon-kvs-producer-sdk-cpp \
    python3-pyserial \
    gst-variable-rtsp-server \
    python3-ar0234 \
    curl \
    git \
    ser2net \
    ffmpeg \
    python3-pycairo \
    python3-pip \
    python3-periphery \
"

CORE_IMAGE_EXTRA_INSTALL += " \
    packagegroup-fsl-gstreamer1.0-debug \
    packagegroup-fsl-gstreamer1.0-full \
    packagegroup-fsl-gstreamer1.0-commercial \
    linux-firmware \
    firmware-nxp-wifi \
    packagegroup-meta-python \
    gstreamer1.0-python \
"

TARGET_HOSTNAME ?= "scailx-ai"

# Allow build with or without a specific kernel
IMAGE_CONTAINER_NO_DUMMY = "1"

# Workaround /var/volatile for now
ROOTFS_POSTPROCESS_COMMAND += "rootfs_fixup_var_volatile ; "
rootfs_fixup_var_volatile () {
    install -m 1777 -d ${IMAGE_ROOTFS}/${localstatedir}/volatile/tmp
    install -m 755 -d ${IMAGE_ROOTFS}/${localstatedir}/volatile/log
}
