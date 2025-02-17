FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://libgstpython.so"

# for some reason the libgstpython so doesnt work as built from yocto.
# building on device works, which is what this binary is. another option
# is to add the libgstpython.so to ld_preload, either in /etc/ld.so.preload or /etc/profile script.
do_install:append() {
    install -d ${D}${libdir}/gstreamer-1.0
    install -m 0644 ${WORKDIR}/libgstpython.so ${D}${libdir}/gstreamer-1.0/
}