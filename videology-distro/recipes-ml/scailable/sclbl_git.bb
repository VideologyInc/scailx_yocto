SUMMARY = "Scailable AI runtime"
LICENSE = "Proprietary"
HOMEPAGE = "https://github.com/scailable/sclbl-tutorials"
LIC_FILES_CHKSUM = "file://${WORKDIR}/license-agreement-03112021.pdf;md5=d918cd28f640185b2685dade2451c197"

archurl = ""
special = ""
special:mx8-camera = "-videology"
archurl:aarch64    = "arm64"
archurl:arm64      = "arm64"
archurl:armv7      = "arm32"
archurl:x86_64     = "x86_64"

PV = "2.3.220"
SRC_URI = "https://download.sclbl.net/lonelinux-sclbl-${archurl}${special}.tgz;name=${archurl}${special};downloadfilename=${PN}-${PV}.tar.gz"
SRC_URI[arm64.sha256sum] = "c78cc4a616c2bc6a556c770d2aa17cd3ed2024a2743ff9f70dac381439d7a9d7"
SRC_URI[arm64-videology.sha256sum] = "c78cc4a616c2bc6a556c770d2aa17cd3ed2024a2743ff9f70dac381439d7a9d7"

SRC_URI += "https://github.com/scailable/sclbl-tutorials/raw/f788e4c399e3fd6cb669d498a6f9fc75968377a1/terms/license-agreement-03112021.pdf;name=lic"
SRC_URI[lic.sha256sum] = "470016616c4cee9ee922efa6ad58e46f7fe3c244b93bce1ac789bbfc08343da8"

# SRC_URI += "file://defaults"

COMPATIBLE_MACHINE = "(x86_64|aarch64|arm)"

S = "${WORKDIR}/sclbl"

do_configure[noexec] = "1"
do_compile[noexec] = "1"

INSTALL_DIR='/opt/sclbl'

do_install () {
    install -d ${D}${INSTALL_DIR}/etc
    cp -r ${S}/* ${D}${INSTALL_DIR}
    # install -m 0755 ${WORKDIR}/defaults ${D}${INSTALL_DIR}/etc/settings
    chmod +x ${D}${INSTALL_DIR}/bin/*
    chmod +x ${D}${INSTALL_DIR}/etc/init
    chmod +x ${D}${INSTALL_DIR}/etc/install
}

FILES:${PN} = "${INSTALL_DIR}/*"
INSANE_SKIP:${PN} = "file-rdeps already-stripped ldflags"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"
