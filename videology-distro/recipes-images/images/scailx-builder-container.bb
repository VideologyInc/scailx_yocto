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

OCI_IMAGE_TAG = "${SCAILX_VERSION}"

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
    splash \
    weston \
    tools-testapps \
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
        imx-test \
        wget curl \
        coreutils \
        cmake \
"

IMAGE_INSTALL += " \
    ${IMAGE_INSTALL_PKCS11TOOL} \
"

IMAGE_INSTALL_PKCS11TOOL = " \
    opensc pkcs11-provider pkcs11-helper se05x-pkcs11 \
    packagegroup-security-tpm2 \
    swtpm softhsm \
    ${@bb.utils.contains('MACHINE_FEATURES', 'optee', 'optee-client optee-os', '', d)}"

IMAGE_INSTALL += " \
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
    bun \
    go go-runtime\
    nginx \
    fluentbit \
    zabbix \
    htop \
    clang \
    i2c-tools \
    pyhailort \
    hailo-pci \
    hailortcli \
    libgsthailo \
    libhailort \
    packagegroup-hailo-tappas-dev-pkg \
    serial-xfer \
    imx-g2d-samples \
    imx-m7-demos \
    imx-gpu-viv-tools \
    imx-gpu-viv-demos \
    python3-pyserial \
    gst-plugin-webrtc \
    libsoup-2.4 \
    libusbgx \
    libuvc \
    json-glib \
    tvm \
    openssl \
    paho-mqtt-c \
    flatbuffers \
    fluentbit \
    protobuf \
    gst-variable-rtsp-server \
    python3-ar0234 \
    curl \
    git \
    nano \
    micro \
    ca-certificates \
    ser2net \
    iwd \
    impala \
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
    libcxx-dev libcxx-staticdev compiler-rt-dev libstdc++-dev libc6-dbg isp-imx-dbg \
    boost-dbg gcc-runtime-dbg glibc-mtrace-dbg imx-gpu-g2d-dev libg2d-dbg libjsoncpp-dbg libdrm-dev libgcc-dev libstdc++-dev libtinyxml2-dbg linux-imx-headers-dbg \
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
