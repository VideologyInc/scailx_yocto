LICENSE = "CLOSED"
SECTION = "multimedia"

SRC_URI = "git://git@github.com/VideologyInc/scailx-embedded.git;protocol=ssh;branch=develop"

# Modify these as desired
PV = "24.08.1"
SRCREV = "a59d5d3a38f80564db1b449a27fb3ea2636d425a"
S = "${WORKDIR}/git"
B = "${WORKDIR}/build"

PKGCONFIG = "infer gstosd events test mqttsink apps"
DEPENDS += "gstreamer1.0 opencv gstreamer1.0-plugins-bad libsoup-2.4 json-glib tensorflow-lite tensorflow-lite-vx-delegate tvm paho-mqtt-c flatbuffers flatbuffers-native protobuf protobuf-native"
RDEPENDS:${PN} += "python3-core gstreamer1.0-plugins-bad-webrtc"

inherit pkgconfig meson use-imx-headers

COMPATIBLE_MACHINE = "(mx8-nxp-bsp)"

FILES:${PN} = "${libdir} ${bindir} ${datadir}"

PACKAGES:remove = "${PN}-doc ${PN}-dev ${PN}-locale ${PN}-staticdev"
PACKAGE_DEBUG_SPLIT_STYLE = 'debug-without-src'

RDEPENDS:${PN} += "python3-core"
FILES:${PN} += "${datadir}/${PN}"

# RM_WORK_EXCLUDE += "${PN}"
