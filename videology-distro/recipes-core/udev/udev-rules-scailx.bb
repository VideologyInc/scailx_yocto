DESCRIPTION = "udev rules for scailx hardware"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit allarch

SRC_URI += " file://scailx-symlinks.rules"
SRC_URI += " file://sd-mount.rules"
SRC_URI += " file://usb-mount.rules"
SRC_URI += " file://v4l2.rules"
SRC_URI += " file://video-links.rules"
S = "${WORKDIR}"

PACKAGES =+ "${PN}-mount"

do_install () {
	install -d ${D}${sysconfdir}/udev/rules.d
	install -m 0644 ${WORKDIR}/scailx-symlinks.rules ${D}${sysconfdir}/udev/rules.d/
	install -m 0644 ${WORKDIR}/sd-mount.rules        ${D}${sysconfdir}/udev/rules.d/
	install -m 0644 ${WORKDIR}/usb-mount.rules       ${D}${sysconfdir}/udev/rules.d/
	install -m 0644 ${WORKDIR}/v4l2.rules            ${D}${sysconfdir}/udev/rules.d/
	install -m 0644 ${WORKDIR}/video-links.rules     ${D}${sysconfdir}/udev/rules.d/
}

FILES:${PN}-mount = "${sysconfdir}/udev/rules.d/sd-mount.rules ${sysconfdir}/udev/rules.d/usb-mount.rules"
