LICENSE = "CLOSED"
SECTION = "multimedia"

SRC_URI = "git://git@github.com/VideologyInc/scailx-embedded.git;protocol=ssh;branch=develop"

# Modify these as desired
PV = "0.1.0"
SRCREV = "e9fbd5074b85bc2ca90540b79274b3f38bf33f81"
S = "${WORKDIR}/git"
B = "${WORKDIR}/build"

PKGCONFIG = "infer gstosd events test mqttsink apps"
DEPENDS += "gstreamer1.0 opencv gstreamer1.0-plugins-bad libsoup-2.4 json-glib tensorflow-lite-vx-delegate paho-mqtt-c"

inherit pkgconfig meson use-imx-headers

COMPATIBLE_MACHINE = "(mx8-nxp-bsp)"

FILES:${PN} = "${libdir} ${bindir} ${datadir}"

# INSANE_SKIP:${PN} += "installed-vs-shipped"
PACKAGES:remove = "${PN}-doc ${PN}-dev ${PN}-locale ${PN}-staticdev"

RDEPENDS:${PN} += "python3-core"
# FILES:${PN} += "${datadir}/${PN}"
# FILES:${PN}-dev = "${libdir}/pkgconfig"

RM_WORK_EXCLUDE += "${PN}"
