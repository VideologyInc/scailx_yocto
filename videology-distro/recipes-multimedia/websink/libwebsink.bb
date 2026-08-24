DESCRIPTION = "Gstreamer webrtc sink plugin"
LICENSE = "CLOSED"

SRCNAME = "websink"

RDEPENDS:${PN} = "gstreamer1.0 gobject-introspection glib-2.0 python3"

SRC_URI += "${SCAILX_ARTIFACTS_URI}/websink/libwebsink.so"
SRC_URI[sha256sum] = "d4c40dc7f08de4aedc3ef84e0dc4a26570f0564f2ce473383283d965cf4fdd90"
SRC_URI += "file://websink_wrapper.py"

PV = "0.1.0"

do_compile[noexec] = "1"
INSANE_SKIP:${PN} += "already-stripped"

do_install(){
    install -d ${D}${libdir}/gstreamer-1.0/python/
    install -D -m 0755 ${WORKDIR}/websink_wrapper.py ${D}${libdir}/gstreamer-1.0/python/
    install -D -m 0755 ${WORKDIR}/libwebsink.so      ${D}${libdir}/gstreamer-1.0/
}

FILES:${PN} += "${libdir}/gstreamer-1.0/"
