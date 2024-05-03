# A desktop image with an Desktop rootfs
#
# Note that we have a tight dependency to ubuntu-base
# and that we cannot just install arbitrary Yocto packages to avoid
# rootfs pollution or destruction.
PV = "${@d.getVar('PREFERRED_VERSION_ubuntu-base', True) or '1.0'}"

require fsl-desktop-image-common.inc

# inherit image-oci
inherit scailx-swupdate-image

DEFAULT_DTB = "scailx_karo_cameras.dtb"

IMAGE_FSTYPES += "tar.gz"

RM_WORK_EXCLUDE += "${PN}"

OVERLAYFS_QA_SKIP[storage] = "mount-configured"

ROOTFS_POSTPROCESS_COMMAND:append = " do_save_cheese; do_rm_opencv_test_and_sample; "
IMAGE_PREPROCESS_COMMAND:append = " do_change_inputrc; do_trim_usr_dirs; "

REQUIRED_DISTRO_FEATURES = "wayland"

ML_NNSTREAMER_PKGS = ""

# nnstreamer
# nnstreamer-tensorflow-lite
# nnstreamer-python3
# nnstreamer-protobuf

# This must be added first as it provides the foundation for
# subsequent modifications to the rootfs
IMAGE_INSTALL += "\
	ubuntu-base \
	ubuntu-base-dev \
	ubuntu-base-dbg \
	ubuntu-base-doc \
"

# chromium-ozone-wayland

IMAGE_INSTALL += "\
	firmwared \
	systemd-gpuconfig \
	alsa-state \
	libcogl \
	clutter-gst-3.0 \
	cheese \
	xwayland \
	gtk+3-gles \
	tensorflow-lite \
	tensorflow-lite-vx-delegate \
	${ML_NNSTREAMER_PKGS} \
	opencv \
        udev-extraconf \
"
APTGET_EXTRA_PACKAGES += "\
	ntpdate patchelf \
	libcairo2 libpixman-1-0 libpango-1.0-0 libpangocairo-1.0-0 \
	squashfs-tools \
	nano v4l-utils python3-periphery i2c-tools libarchive13 libconfig9 libev4 libfdisk1 libjson-c5 liblua5.3-0 librsync2 libswupdate0.1 liburiparser1 libuv1 libwebsockets16 \
"
# v4l-utils    python3-pyserial    python3-periphery    i2ctools
# golang-github-snapcore-snapd-dev golang-github-ubuntu-core-snappy-dev snap-confine snapd-xdg-open snapd ubuntu-core-launcher ubuntu-core-snapd-units ubuntu-snappy-cli ubuntu-snappy libqt5gui5-gles libqt5quick5-gles qtwayland5

##############################################################################
# NOTE: We cannot install arbitrary Yocto packages as they will
# conflict with the content of the prebuilt Desktop rootfs and pull
# in dependencies that may break the rootfs.
# Any package addition needs to be carefully evaluated with respect
# to the final image that we build.
##############################################################################

IMAGE_INSTALL += " \
    packagegroup-fsl-gstreamer1.0 \
    packagegroup-fsl-gstreamer1.0-full \
"

IMAGE_INSTALL:remove = "kernel-devicetree"

# GPU driver
G2D_SAMPLES                 = ""
# G2D_SAMPLES:imxgpu2d        = "imx-g2d-samples"
# G2D_SAMPLES:imxdpu          = "imx-g2d-samples"

IMAGE_INSTALL += " \
    wayland-protocols \
    libclc-imx libclc-imx-dev \
    libgles1-imx libgles1-imx-dev \
    libgles2-imx libgles2-imx-dev \
    libgles3-imx-dev \
    libglslc-imx \
    libopencl-imx \
    libegl-imx libegl-imx-dev \
    libgal-imx libgal-imx-dev \
    libvsc-imx \
    libgbm-imx libgbm-imx-dev \
    libvulkan-imx \
    libopenvx-imx libopenvx-imx-dev \
    libnn-imx \
    libdrm-vivante \
    imx-gpu-viv-tools \
    libgpuperfcnt \
    ${G2D_SAMPLES} \
    apitrace \
    gputop \
"
# isp
IMAGE_INSTALL:append:mx8mp-nxp-bsp = " \
    isp-imx \
    basler-camera \
    kernel-module-isp-vvcam \
"

IMAGE_INSTALL:remove:mx8mm-nxp-bsp = " \
    libgles3-imx-dev \
    libclc-imx libclc-imx-dev \
    libopencl-imx \
    libvulkan-imx \
    libopenvx-imx libopenvx-imx-dev \
    libnn-imx \
    tensorflow-lite \
    tensorflow-lite-vx-delegate \
    ${ML_NNSTREAMER_PKGS} \
"

APTGET_EXTRA_PACKAGES_DESKTOP:scailx-imx8mp = ""

fakeroot do_save_cheese() {
	set -x

	if [ -e "${IMAGE_ROOTFS}/usr/bin/cheese" ]; then
		# backup cheese bin
		mv ${IMAGE_ROOTFS}/usr/bin/cheese ${IMAGE_ROOTFS}/usr/bin/cheese_imx
	fi

	set +x
}

do_rm_opencv_test_and_sample() {
	set -x

	rm -r ${IMAGE_ROOTFS}/usr/share/opencv4 || echo "opencv4 not found"
	rm -r ${IMAGE_ROOTFS}/usr/share/OpenCV || echo "OpenCV not found"

	set +x
}

do_change_inputrc() {
    set -x

    # Change the default inputrc to use vi mode
    echo '"\\e[A": history-search-backward' >> ${IMAGE_ROOTFS}/etc/inputrc
    echo '"\\e[B": history-search-forward' >> ${IMAGE_ROOTFS}/etc/inputrc
    echo '"\\e[C": forward-char' >> ${IMAGE_ROOTFS}/etc/inputrc
    echo '"\\e[D": backward-char' >> ${IMAGE_ROOTFS}/etc/inputrc
    ln -sf -T /usr/bin/busybox ${IMAGE_ROOTFS}/usr/bin/sh

    set +x
}

do_trim_usr_dirs() {
	set -x

	rm -r ${IMAGE_ROOTFS}/usr/share/backgrounds
	rm -r ${IMAGE_ROOTFS}/usr/share/doc

	set +x
}