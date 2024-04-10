#
# Copyright (c) Videology, 2024
#
# SPDX-License-Identifier: MIT

DESCRIPTION = "Systemd monitoring script which checks up swupdate and toggles the system status LED"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit allarch systemd

SRC_URI += "file://scailx-i2c-overlay.sh \
			file://scailx-i2c-overlay.service \
			"
S = "${WORKDIR}"

SYSTEMD_SERVICE:${PN} = "scailx-i2c-overlay.service"
SYSTEMD_AUTO_ENABLE = "enable"
do_install() {
   	install -D -m0644 "${WORKDIR}/scailx-i2c-overlay.service" "${D}${systemd_system_unitdir}/scailx-i2c-overlay.service"
	install -D -m0755 "${WORKDIR}/scailx-i2c-overlay.sh" "${D}${bindir}/scailx-i2c-overlay.sh"
}

RDEPENDS:${PN} = "bash i2c-tools"
SYSTEMD_SERVICE:${PN} = "scailx-i2c-overlay.service"
FILES:${PN} = "${systemd_system_unitdir}/ ${bindir}"
