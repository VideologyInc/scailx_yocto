SUMMARY = "Gstreamer Daemon 1.0"
DESCRIPTION = "GStreamer framework for controlling audio and video streaming using TCP connection messages"
HOMEPAGE = "https://developer.ridgerun.com/wiki/index.php?title=Gstd-1.0"
SECTION = "multimedia"
LICENSE = "GPLv2+"

LIC_FILES_CHKSUM = "file://COPYING;md5=c71b653a0f608a58cdc5693ae57126bc"

DEPENDS = "gstreamer1.0 gstreamer1.0-plugins-base gstreamer1.0-plugins-bad gstreamer1.0-rtsp-server json-glib libdaemon jansson libsoup-2.4"

SRCBRANCH ?= "master"
SRCREV = "dac03d5e4102080a8f3ef274a6a6583455962095"
SRC_URI = "git://github.com/RidgeRun/gstd-1.x.git;protocol=https;branch=${SRCBRANCH} \
           "

SRC_URI += "file://gstd.service"

S = "${WORKDIR}/git"

FILES_${PN} += "/run \
                /var/log \
                ${libdir}/libgstd-1.0.so \
                ${libdir}/libgstc-1.0.so \
                "

EXTRA_OEMESON = "-Dwith-gstd-runstatedir=/run \
                -Dwith-gstd-logstatedir=/var/log/ \
                "

SYSTEMD_SERVICE:${PN} = "gstd.service"
SYSTEMD_AUTO_ENABLE = "disable"
FILES:${PN} += "${systemd_user_unitdir}"

# -Denable-python=false

PACKAGES =+ "${PN}-python3"
FILES:${PN}-python3 += "${PYTHON_SITEPACKAGES_DIR}"
RDEPENDS:${PN}-python3 += "${PN}"

do_install:append() {
    rm -rf ${D}/run    # Your code here
    rm -rf ${D}${systemd_user_unitdir}
    install -d ${D}${systemd_system_unitdir}
    cp ${WORKDIR}/gstd.service ${D}${systemd_system_unitdir}/gstd.service
}

inherit meson pkgconfig gettext python3native systemd