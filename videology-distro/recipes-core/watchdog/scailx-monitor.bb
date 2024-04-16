#
# Copyright (c) Videology, 2024
#
# SPDX-License-Identifier: MIT

DESCRIPTION = "Systemd monitoring script which checks up swupdate and toggles the system status LED"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit allarch systemd

SRC_URI += "file://scailx-monitor-d.py \
			file://scailx-monitor.service \
			"
S = "${WORKDIR}"

SYSTEMD_SERVICE:${PN} = "scailx-monitor.service"
SYSTEMD_AUTO_ENABLE = "enable"
do_install() {
   	install -D -m0644 "${WORKDIR}/scailx-monitor.service" "${D}${systemd_system_unitdir}/scailx-monitor.service"
	install -D -m0755 "${WORKDIR}/scailx-monitor-d.py" "${D}${bindir}/scailx-monitor-d"
}

SYSTEMD_SERVICE:${PN} = "scailx-monitor.service"
FILES:${PN} = "${systemd_system_unitdir}/ ${bindir}"
