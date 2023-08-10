SUMMARY = "V4L2Loopback"
DESCRIPTION = "v4l2loopback module"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263"

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
    ${CC} ${CFLAGS} ${LDFLAGS} -Wall utils/v4l2loopback-ctl.c -I. -o ${PN}-ctl
}

do_install:append () {
    install -d ${D}/${bindir}
    install -m 755 ${S}/${PN}-ctl ${D}/${bindir}
}

RPROVIDES:${PN} += "kernel-module-${PN}"

FILES:${PN}-utils += "${bindir}/${PN}-ctl"

KERNEL_MODULE_AUTOLOAD = "v4l2loopback"
KERNEL_MODULE_PROBECONF = "v4l2loopback"
module_conf_v4l2loopback = "options v4l2loopback video_nr=10,11,12 card_label=loop1,loop2,loop3 exclusive_caps=1"
