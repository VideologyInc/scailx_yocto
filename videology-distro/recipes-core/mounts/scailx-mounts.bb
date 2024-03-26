SUMMARY = "make systemd services to mount persistent volumes for scailx"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI += "file://boot.mount"
SRC_URI += "file://storage.mount"

inherit allarch systemd

PACKAGES += "${PN}-boot ${PN}-storage"

FILES:${PN}-boot = "${systemd_system_unitdir}/boot.mount /boot"
FILES:${PN}-storage = "${systemd_system_unitdir}/storage.mount /storage"

do_install() {
    install -d "${D}${systemd_system_unitdir}"
    install -d "${D}/boot"
    install -d "${D}/storage"
    install -m 644 "${WORKDIR}/boot.mount" "${D}${systemd_system_unitdir}"
    install -m 644 "${WORKDIR}/storage.mount" "${D}${systemd_system_unitdir}"
}

SYSTEMD_PACKAGES = "${PN}-boot ${PN}-storage"
SYSTEMD_SERVICE:${PN}-storage = "storage.mount"
SYSTEMD_SERVICE:${PN}-boot    = "boot.mount"
