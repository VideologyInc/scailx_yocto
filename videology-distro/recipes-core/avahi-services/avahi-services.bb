SUMMARY = "simple avahi services for scailx"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI += "file://sftp-ssh.service "
SRC_URI += "file://ssh.service "


PACKAGES = "${PN}-ssh ${PN}-sftp"

FILES:${PN}-ssh = "${sysconfdir}/avahi/services/ssh.service"
FILES:${PN}-sftp = "${sysconfdir}/avahi/services/sftp-ssh.service"

do_install() {
    install -d "${D}${sysconfdir}/avahi/services/"
    install -Dm 644 "${WORKDIR}/sftp-ssh.service" "${D}${sysconfdir}/avahi/services/sftp-ssh.service"
    install -Dm 644 "${WORKDIR}/ssh.service"      "${D}${sysconfdir}/avahi/services/ssh.service"
}
