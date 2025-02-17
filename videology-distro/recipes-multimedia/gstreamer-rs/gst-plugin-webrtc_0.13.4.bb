SUMMARY = "GStreamer Rust plugin webrtc"
DESCRIPTION = "Rust plugin for webrtcbin"
HOMEPAGE = "https://gitlab.freedesktop.org/gstreamer/gst-plugins-rs"
LICENSE = "MPL-2.0"
LIC_FILES_CHKSUM = "file://LICENSE-MPL-2.0;md5=815ca599c9df247a0c7f619bab123dad"

inherit pkgconfig cargo cargo-update-recipe-crates

SRC_URI = "crate://crates.io/${BPN}/${PV}"
SRC_URI[gst-plugin-webrtc-0.13.4.sha256sum] = "5a1d9d2cdcd92e08d4c0ff14aedebb9cc55dab1de9249e2274581f05370190e6"

require ${BPN}-crates.inc

export CARGO_NET_GIT_FETCH_WITH_CLI = "true"

CARGO_FEATURES ?= "janus livekit v1_22 whip"
CARGO_BUILD_FLAGS += "--features '${CARGO_FEATURES}'"

DEPENDS = "gstreamer1.0 \
           gstreamer1.0-plugins-base \
           gstreamer1.0-plugins-bad \
           openssl \
"

RDEPENDS_${PN} += "gstreamer1.0-plugins-bad-webrtc libgstwebrtc-1.0-0 gstreamer1.0-plugins-base"
CARGO_INSTALL_LIBRARIES = "libgstrswebrtc.so"

do_install:append() {
    install -d ${D}${libdir}/gstreamer-1.0
    mv ${D}${rustlibdir}/${CARGO_INSTALL_LIBRARIES} ${D}${libdir}/gstreamer-1.0/${CARGO_INSTALL_LIBRARIES}
}

FILES:${PN} += "${libdir}/gstreamer-1.0/"