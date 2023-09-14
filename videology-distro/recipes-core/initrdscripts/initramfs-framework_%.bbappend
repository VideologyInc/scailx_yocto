FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://swupdate "
SRC_URI:append = " file://storage "
SRC_URI:append = " file://pstree "
SRC_URI:append = " file://i2cdetect file://cam-overlays "

PACKAGES  += " initramfs-module-swupdate "
PACKAGES  += " initramfs-module-storage "
PACKAGES  += " initramfs-module-i2cdetect "

do_install:append() {
    # swupdate
    install -m 0755 ${WORKDIR}/swupdate ${D}/init.d/50-swupdate
    install -m 0755 ${WORKDIR}/swupdate ${D}/init.d/99-swupdate
    # storage
    install -m 0755 ${WORKDIR}/storage ${D}/init.d/60-storage
    # install -m 0755 ${WORKDIR}/storage-overlay ${D}/init.d/90-storage-overlay
    # i2cdetect
    install -m 0755 ${WORKDIR}/i2cdetect ${D}/init.d/65-i2cdetect
    install -d ${D}${sysconfdir}/
    install -m 0644 ${WORKDIR}/cam-overlays ${D}${sysconfdir}/
    # pstree
    install -d ${D}${base_bindir}/
    install -m 0755 ${WORKDIR}/pstree ${D}${base_bindir}/pstree
}

SUMMARY:initramfs-module-swupdate = "initramfs support for swupdate"
RDEPENDS:initramfs-module-swupdate = "swupdate swupdate-www busybox-udhcpc busybox"
FILES:initramfs-module-swupdate = "/init.d/50-swupdate /init.d/99-swupdate ${base_bindir}/pstree"

SUMMARY:initramfs-module-storage = "initramfs support for storage partition"
RDEPENDS:initramfs-module-storage = "${PN}-base"
FILES:initramfs-module-storage = "/init.d/60-storage "

SUMMARY:initramfs-module-i2cdetect = "initramfs support for autoloading devicetree-overlays based on I2C devices and overlay file"
RDEPENDS:initramfs-module-i2cdetect = "${PN}-base i2c-tools udev-rules-scailx"
FILES:initramfs-module-i2cdetect = "/init.d/65-i2cdetect ${sysconfdir}/cam-overlays"

