SUMMARY = "GStreamer Rust plugin webrtc"
DESCRIPTION = "Rust plugin for webrtcbin"
HOMEPAGE = "https://github.com/videologyinc/websink"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b3a0d536af8ffb9b047ff986b81105c3"

inherit pkgconfig cargo cargo-update-recipe-crates

SRC_URI = "git://github.com/videologyinc/websink.git;protocol=https;branch=master"
SRCREV = "c9c61f38431d56507e141fc040019d3e3440ca15"
SRC_URI += "file://websink_wrapper.py"

S = "${WORKDIR}/git"

inherit cargo-update-recipe-crates
inherit cargo_c
require ${BPN}-crates.inc

export CARGO_NET_GIT_FETCH_WITH_CLI = "true"

DEPENDS += " gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad glib-2.0 "

do_install:append(){
    install -d ${D}${libdir}/gstreamer-1.0/python/
    install -D -m 0755 ${WORKDIR}/websink_wrapper.py ${D}${libdir}/gstreamer-1.0/python/
}

FILES:${PN} += " ${libdir}/gstreamer-1.0 "