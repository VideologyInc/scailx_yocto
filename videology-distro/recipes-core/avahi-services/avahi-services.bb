SUMMARY = "simple avahi services for scailx"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI += "file://sftp-ssh.service "
SRC_URI += "file://ssh.service "
SRC_URI += "file://swupdate.service "

inherit allarch
PACKAGES = "${PN}-ssh ${PN}-sftp ${PN}-swupdate"

FILES:${PN}-ssh = "${sysconfdir}/avahi/services/ssh.service"
FILES:${PN}-sftp = "${sysconfdir}/avahi/services/sftp-ssh.service"
FILES:${PN}-swupdate = "${sysconfdir}/avahi/services/swupdate.service"

do_install() {
    install -d "${D}${sysconfdir}/avahi/services/"
    install -Dm 644 "${WORKDIR}/sftp-ssh.service" "${D}${sysconfdir}/avahi/services/sftp-ssh.service"
    install -Dm 644 "${WORKDIR}/ssh.service"      "${D}${sysconfdir}/avahi/services/ssh.service"
    install -Dm 644 "${WORKDIR}/swupdate.service" "${D}${sysconfdir}/avahi/services/swupdate.service"
}
