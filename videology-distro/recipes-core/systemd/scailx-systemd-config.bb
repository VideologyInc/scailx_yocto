#
# Copyright (c) Videology, 2024
#
# SPDX-License-Identifier: MIT

DESCRIPTION = "Systemd config to start and maintain watchdog."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit allarch

SRC_URI += "file://99-watchdog.conf"
SRC_URI += "file://99-shutdown.conf"

S = "${WORKDIR}"

do_install() {
   	install -D -m0644 "${WORKDIR}/99-watchdog.conf" "${D}${systemd_unitdir}/system.conf.d/99-watchdog.conf"
   	install -D -m0644 "${WORKDIR}/99-shutdown.conf" "${D}${systemd_unitdir}/system.conf.d/99-shutdown.conf"
}

FILES:${PN} = "${systemd_unitdir}/"
ALLOW_EMPTY:${PN} = "1"


