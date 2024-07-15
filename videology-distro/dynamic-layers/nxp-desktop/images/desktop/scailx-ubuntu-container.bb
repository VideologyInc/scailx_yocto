# A desktop image with an Desktop rootfs
#
# Note that we have a tight dependency to ubuntu-base
# and that we cannot just install arbitrary Yocto packages to avoid
# rootfs pollution or destruction.
PV = "${@d.getVar('PREFERRED_VERSION_ubuntu-base', True) or '1.0'}"

require ubuntu-image-container.inc

IMAGE_FSTYPES = "docker-archive.xz"

IMAGE_FEATURES = ""
IMAGE_LINGUAS = ""
NO_RECOMMENDATIONS = "1"
MACHINE_FEATURES = ""

RM_WORK_EXCLUDE += "${PN}"

# This must be added first as it provides the foundation for
# subsequent modifications to the rootfs
IMAGE_INSTALL += "\
	ubuntu-base \
	ubuntu-base-dev \
"

IMAGE_INSTALL += "swupdate-src swupdate-dev"

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

# isp
IMAGE_INSTALL:append:mx8mp-nxp-bsp = " \
    isp-imx \
"

# AI demo
IMAGE_INSTALL += " scailx-tflite-demo "