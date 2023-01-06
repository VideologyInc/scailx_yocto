SUMMARY = "Scailable AI runtime"
LICENSE = "Proprietary"
HOMEPAGE = "https://github.com/scailable/sclbl-tutorials"
LIC_FILES_CHKSUM = "file://${WORKDIR}/license-agreement-03112021.pdf;md5=d918cd28f640185b2685dade2451c197"

ARCHURL = ""
ARCHURL:mx8-camera = "arm64-videology"
ARCHURL:aarch64    = "arm64"
ARCHURL:arm64      = "arm64"
ARCHURL:armv7      = "arm32"
ARCHURL:x86_64     = "x86_64"

PV = "2.2.51"
SRCREV = "f788e4c399e3fd6cb669d498a6f9fc75968377a1"
SRC_URI = "https://github.com/scailable/sclbl-tutorials/raw/${SRCREV}/downloads/files/lonelinux-sclbl-${ARCHURL}.tgz;name=${ARCHURL};downloadfilename=${PN}-${PV}.tar.gz"
SRC_URI[arm64.sha256sum] = "ca03ada3b41d5f57bfe3821600f20f31af6596d422a366a3dcd3402cf384c069"
SRC_URI[arm64-videology.sha256sum] = "1a4132eb2d88006e0077dc6404d8093b5eab2754fe53a86abefac7f3f55c4285"

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

FILES:${PN} = "${INSTALL_DIR}/*"
INSANE_SKIP:${PN} = "file-rdeps already-stripped ldflags"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"
