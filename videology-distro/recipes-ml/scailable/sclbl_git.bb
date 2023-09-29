SUMMARY = "Scailable AI runtime"
LICENSE = "Proprietary"
HOMEPAGE = "https://github.com/scailable/sclbl-tutorials"
LIC_FILES_CHKSUM = "file://${WORKDIR}/license-agreement-03112021.pdf;md5=d918cd28f640185b2685dade2451c197"

archurl = ""
special = ""
special:scailx-imx8mp = "_videology"
archurl:aarch64    = "aarch64"
archurl:arm64      = "aarch64"
archurl:armv7      = "arm32"
archurl:x86_64     = "x86_64"

PV = "3.0.84"
SRC_URI = "https://download.sclbl.net/sclbl-${archurl}${special}.tgz;name=${archurl}${special};downloadfilename=${PN}-${PV}.tar.gz"
SRC_URI[aarch64.sha256sum] = "143c2d162e39f2ae8024cb7171f61b6cbfbf831e0615e6d1ca070ff4fc332935"
SRC_URI[aarch64_videology.sha256sum] = "143c2d162e39f2ae8024cb7171f61b6cbfbf831e0615e6d1ca070ff4fc332935"

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
