DESCRIPTION = "The Edge TPU runtime for Google's Coral chip"
SECTION = "libs"
LICENSE = "Proprietary"
LIC_FILES_CHKSUM = "file://${PN}/LICENSE.txt;md5=c0e85c67b919e863a1a7a3da109dc40d"

DEPENDS = "xz"

SRC_URI = "https://dl.google.com/coral/edgetpu_api/edgetpu_runtime_20210119.zip"
SRC_URI[sha256sum] = "b23b2c5a227d7f0e65dcc91585028d27c12e764f8ce4c4db3f114be4a49af3ae"

S = "${WORKDIR}/edgetpu_runtime"

FILES_${PN} = "${libdir}/libedgetpu.so.*"

INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"

do_install() {
    install -d 0755 ${D}${libdir}
    install -m 0644 ${PN}/throttled/${HOST_ARCH}/libedgetpu.so.* ${D}${libdir}
}
