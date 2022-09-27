SUMMARY = "Scailable AI runtime"
LICENSE = "Proprietary"
HOMEPAGE = "https://github.com/scailable/sclbl-tutorials"
LIC_FILES_CHKSUM = "file://${WORKDIR}/license-agreement-03112021.pdf;md5=d918cd28f640185b2685dade2451c197"

ARCHURL ?= ""
ARCHURL_aarch64 = "arm64"
ARCHURL_arm64   = "arm64"
ARCHURL_armv7   = "arm32"
ARCHURL_x86_64  = "x86_64"

PV = "2.2.30"
SRCREV = "d9fd55c28556f7973dc7b176b7e65288facd885b"
SRC_URI = "https://github.com/scailable/sclbl-tutorials/raw/${SRCREV}/downloads/files/lonelinux-sclbl-${ARCHURL}.tgz;name=${ARCHURL};downloadfilename=${PN}-${PV}.tar.gz"
SRC_URI[arm64.sha256sum] = "ca03ada3b41d5f57bfe3821600f20f31af6596d422a366a3dcd3402cf384c069"

SRC_URI += "https://github.com/scailable/sclbl-tutorials/raw/${SRCREV}/terms/license-agreement-03112021.pdf;name=lic"
SRC_URI[lic.sha256sum] = "470016616c4cee9ee922efa6ad58e46f7fe3c244b93bce1ac789bbfc08343da8"

SRC_URI += "file://defaults"


COMPATIBLE_MACHINE = "(x86_64|aarch64|arm)"

S = "${WORKDIR}/sclbl"

do_configure[noexec] = "1"
do_compile[noexec] = "1"

INSTALL_DIR='/opt/sclbl'

do_install () {
    install -d ${D}${INSTALL_DIR}/etc
    cp -r ${S}/* ${D}${INSTALL_DIR}
    install -m 0755 ${WORKDIR}/defaults ${D}${INSTALL_DIR}/etc/settings
    chmod +x ${D}${INSTALL_DIR}/bin/*
    chmod +x ${D}${INSTALL_DIR}/etc/init
    chmod +x ${D}${INSTALL_DIR}/etc/install
}

FILES_${PN} = "${INSTALL_DIR}/*"
INSANE_SKIP_${PN} = "file-rdeps already-stripped ldflags"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"
