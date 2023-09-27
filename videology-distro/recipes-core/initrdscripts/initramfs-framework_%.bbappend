FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://swupdate "
SRC_URI:append = " file://storage "
SRC_URI:append = " file://pstree "
SRC_URI:append = " file://i2cdetect file://cam-overlays "
# SRC_URI:append = " file://disk-format "
SRC_URI:append = " file://cryptfs file://cryptfs_pkcs11 file://cryptfs_tpm2 "

PACKAGES  += " initramfs-module-swupdate "
PACKAGES  += " initramfs-module-storage "
PACKAGES  += " initramfs-module-i2cdetect "
PACKAGES  += " initramfs-module-cryptfs "
PACKAGES  += " initramfs-module-cryptfs-pkcs11 "
PACKAGES  += " initramfs-module-cryptfs-tpm2 "

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
    # cryptfs modules
    install -m 0755 ${WORKDIR}/cryptfs ${D}/init.d/999-cryptfs
    install -d ${D}/${sysconfdir}/cryptfs
	install -m 0644 ${WORKDIR}/cryptfs_pkcs11 ${D}/${sysconfdir}/cryptfs/pkcs11
	install -m 0644 ${WORKDIR}/cryptfs_tpm2 ${D}/${sysconfdir}/cryptfs/tpm2
}

SUMMARY:initramfs-module-swupdate = "initramfs support for swupdate"
RDEPENDS:initramfs-module-swupdate = "swupdate swupdate-www busybox-udhcpc busybox"
FILES:initramfs-module-swupdate = "/init.d/50-swupdate /init.d/99-swupdate ${base_bindir}/pstree "
# ${base_bindir}/disk-format"

SUMMARY:initramfs-module-storage = "initramfs support for storage partition"
RDEPENDS:initramfs-module-storage = "${PN}-base util-linux-findfs cryptsetup e2fsprogs-tune2fs p11-kit "
FILES:initramfs-module-storage = "/init.d/60-storage"

SUMMARY:initramfs-module-i2cdetect = "initramfs support for autoloading devicetree-overlays based on I2C devices and overlay file"
RDEPENDS:initramfs-module-i2cdetect = "${PN}-base i2c-tools udev-rules-scailx"
FILES:initramfs-module-i2cdetect = "/init.d/65-i2cdetect ${sysconfdir}/cam-overlays"

SUMMARY:initramfs-module-cryptfs = "initramfs support for encrypted filesystems"
RDEPENDS:initramfs-module-cryptfs = "${PN}-base libgcc e2fsprogs-resize2fs e2fsprogs-e2fsck e2fsprogs-dumpe2fs "
# systemd-crypt"
FILES:initramfs-module-cryptfs = "/init.d/999-cryptfs"

SUMMARY:initramfs-module-cryptfs-pkcs11 = "encrypted filesystems with support for pkcs11"
RDEPENDS:initramfs-module-cryptfs-pkcs11 = "initramfs-module-cryptfs opensc openssl-bin libp11 \
					    ${@bb.utils.contains('MACHINE_FEATURES', 'optee', 'optee-client', '', d)}"
FILES:initramfs-module-cryptfs-pkcs11 = "${sysconfdir}/cryptfs/pkcs11"

SUMMARY:initramfs-module-cryptfs-tpm2 = "encrypted filesystems with support for tpm 2.0"
RDEPENDS:initramfs-module-cryptfs-tpm2 = "initramfs-module-cryptfs \
					  ${@bb.utils.contains('MACHINE_FEATURES', 'tpm2', 'libtss2 libtss2-mu libtss2-tcti-device', '', d)}"
FILES:initramfs-module-cryptfs-tpm2 = "${sysconfdir}/cryptfs/tpm2"
