SUMMARY = "V4L2Loopback"
DESCRIPTION = "v4l2loopback module"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"


# Use local tarball
SRC_URI = " \
    git://github.com/umlaeute/v4l2loopback.git;protocol=https;branch=main \
    file://0001-change-makefile-for-yocto.patch \
"
SRCREV = "9ba7e294ea6f45410a15f192381d4918e8df4f02"

# Make sure our source directory (for the build) matches the directory structure in the tarball
S = "${WORKDIR}/git"

inherit module

export MAKE_TARGETS = "v4l2loopback.ko"

FILES:${PN} += "/usr/lib/modules"

PACKAGES += " ${PN}-utils ${PN}-examples "

do_compile:append () {
    oe_runmake utils
}

do_install:append () {
    export PREFIX="$prefix"
    oe_runmake install-utils DESTDIR="${D}"
}

RPROVIDES:${PN} += "kernel-module-${PN}"

FILES:${PN}-utils += "${bindir}/${PN}-ctl"

KERNEL_MODULE_AUTOLOAD = "v4l2loopback"
KERNEL_MODULE_PROBECONF = "v4l2loopback"
module_conf_v4l2loopback = "options v4l2loopback video_nr=10,11,12 card_label=loop1,loop2,loop3 exclusive_caps=1"
