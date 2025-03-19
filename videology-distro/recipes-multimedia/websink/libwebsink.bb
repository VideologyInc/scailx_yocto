DESCRIPTION = "Gstreamer webrtc sink plugin"
LICENSE = "CLOSED"

SRCNAME = "websink"

RDEPENDS:${PN} = "gstreamer1.0 gobject-introspection glib-2.0 python3"

SRC_URI += "https://scailx-ppa.org/scailx-apps/websink/libwebsink.so"
SRC_URI[sha256sum] = "7ff6a2ea37a0d2b411ee4dd3221159515e0e731f6d8c3bb345237ea779fdcf90"
SRC_URI += "file://websink_wrapper.py"

PV = "1.0.0"

do_compile[noexec] = "1"
INSANE_SKIP:${PN} += "already-stripped"

do_install(){
    install -d ${D}${libdir}/gstreamer-1.0/python/
    install -D -m 0755 ${WORKDIR}/websink_wrapper.py ${D}${libdir}/gstreamer-1.0/python/
    install -D -m 0755 ${WORKDIR}/libwebsink.so      ${D}${libdir}/gstreamer-1.0/
}

FILES:${PN} += "${libdir}/gstreamer-1.0/"