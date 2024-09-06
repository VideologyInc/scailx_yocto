SUMMARY = "Scailx packagefeed  public key"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://scailx-ppa-key.pub.gpg"

inherit allarch

do_install() {
    install -d "${D}${sysconfdir}/pki"
    install -m 0644 ${WORKDIR}/scailx-ppa-key.pub.gpg "${D}${sysconfdir}/pki"
}

FILES_${PN} = "${sysconfdir}/pki/scailx-ppa-key.pub.gpg"