SUMMARY = "GStreamer Rust plugin webrtc"
DESCRIPTION = "Rust plugin for webrtcbin"
HOMEPAGE = "https://github.com/KobusG/websink"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=84f39ec0f1c900532a719dbb9ef7dfd3"

inherit pkgconfig cargo cargo-update-recipe-crates

SRC_URI = "git://github.com/videologyinc/websink.git;protocol=https;branch=master"
SRCREV = "921d3e5f3e9d63177184901dce617daed0381e25"

S = "${WORKDIR}/git"

inherit cargo-update-recipe-crates
inherit cargo_c
require ${BPN}-crates.inc

export CARGO_NET_GIT_FETCH_WITH_CLI = "true"

DEPENDS += " gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad glib-2.0 "

FILES:${PN} += " ${libdir}/gstreamer-1.0 "