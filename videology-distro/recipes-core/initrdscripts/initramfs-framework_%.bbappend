FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://swupdate file://storage file://pstree "

PACKAGES  += " initramfs-module-swupdate "
PACKAGES  += " initramfs-module-storage "

do_install:append() {
    # swupdate
    install -m 0755 ${WORKDIR}/swupdate ${D}/init.d/50-swupdate
    install -m 0755 ${WORKDIR}/swupdate ${D}/init.d/99-swupdate
    install -m 0755 ${WORKDIR}/storage ${D}/init.d/60-storage
    install -d ${D}${base_bindir}/
    install -m 0755 ${WORKDIR}/pstree ${D}${base_bindir}/pstree
}

SUMMARY:initramfs-module-swupdate = "initramfs support for swupdate"
RDEPENDS:initramfs-module-swupdate = "swupdate swupdate-www busybox-udhcpc busybox"
FILES:initramfs-module-swupdate = "/init.d/50-swupdate /init.d/99-swupdate ${base_bindir}/pstree"

SUMMARY:initramfs-module-storage = "initramfs support for storage partitiion"
RDEPENDS:initramfs-module-storage = "${PN}-base"
FILES:initramfs-module-storage = "/init.d/60-storage "
