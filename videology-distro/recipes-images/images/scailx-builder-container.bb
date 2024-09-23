#
# Based on examples from Scott Murray (Building Container Images with
# OpenEmbedded and the Yocto Project) ELCe 2018
#
SUMMARY = "Basic container image"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

IMAGE_FSTYPES = "container docker-archive.xz"
inherit core-image
inherit image-oci

# get rid of 'rootfs' tag
IMAGE_NAME_SUFFIX ?= ""

inherit kernel_devel

IMAGE_FEATURES = " \
    debug-tweaks \
    tools-profile \
    tools-sdk \
    ssh-server-openssh \
    tools-debug \
    dev-pkgs staticdev-pkgs \
    package-management \
    ssh-server-openssh \
    hwcodecs \
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
    scailx-ppa-key \
    packagegroup-fsl-gstreamer1.0 \
    packagegroup-imx-ml \
    packagegroup-fsl-tools-gpu \
    packagegroup-fsl-tools-benchmark \
    packagegroup-fsl-opencv-imx \
"
IMAGE_INSTALL += " \
    packagegroup-self-hosted-sdk \
    packagegroup-rust-sdk-target \
    packagegroup-self-hosted-graphics \
"

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
    fluentbit \
    htop \
    i2c-tools \
    pyhailort \
    hailortcli \
    amazon-kvs-producer-sdk-cpp \
    imx-g2d-samples \
    imx-m7-demos \
    imx-gpu-viv-tools \
    imx-gpu-viv-demos \
    python3-pyserial \
    libsoup-2.4 \
    json-glib \
    tvm \
    openssl \
    paho-mqtt-c \
    flatbuffers \
    protobuf \
    gst-variable-rtsp-server \
    python3-ar0234 \
    curl \
    git \
    nano \
    ca-certificates \
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

do_add_scailx_version () {
    echo "${SCAILX_VERSION}" > ${IMAGE_ROOTFS}${sysconfdir}/scailx-version
}
IMAGE_PREPROCESS_COMMAND += ";do_add_scailx_version;"

TARGET_HOSTNAME ?= "scailx-ai"

# Allow build with or without a specific kernel
IMAGE_CONTAINER_NO_DUMMY = "1"

# Workaround /var/volatile for now
ROOTFS_POSTPROCESS_COMMAND += "rootfs_fixup_var_volatile ; "
rootfs_fixup_var_volatile () {
    install -m 1777 -d ${IMAGE_ROOTFS}/${localstatedir}/volatile/tmp
    install -m 755 -d ${IMAGE_ROOTFS}/${localstatedir}/volatile/log
}
