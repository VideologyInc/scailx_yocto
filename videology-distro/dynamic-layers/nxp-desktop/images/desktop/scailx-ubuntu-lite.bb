# Ubuntu distro image without Desktop rootfs
#
# Note that we have a tight dependency to ubuntu-main
# and that we cannot just install arbitrary Yocto packages to avoid
# rootfs pollution or destruction.
PV = "${@d.getVar('PREFERRED_VERSION_ubuntu-base', True) or '1.0'}"

REQUIRED_DISTRO_FEATURES = ""

require fsl-desktop-image-common.inc

APTGET_EXTRA_PACKAGES_MAIN = "1"

# This must be added first as it provides the foundation for
# subsequent modifications to the rootfs
IMAGE_INSTALL += "\
	ubuntu-main \
"

SOC_DEFAULT_IMAGE_FSTYPES:append = " tar.gz"
SOC_DEFAULT_IMAGE_FSTYPES:remove = " tar.bz2"

# IMAGE_ROOTFS_SIZE ?= "1572864"

# chromium-ozone-wayland

IMAGE_INSTALL += "\
	alsa-state \
    udev-extraconf \
"
APTGET_EXTRA_PACKAGES += "\
	ntpdate patchelf \
	squashfs-tools \
"