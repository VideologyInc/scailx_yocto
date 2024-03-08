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
SRC_URI[aarch64.sha256sum] = "5f075657f460a63ebe0f10161a5d6cfb3256e8311a8ca5c109abc5c623e2f2f4"
SRC_URI[aarch64_videology.sha256sum] = "5f075657f460a63ebe0f10161a5d6cfb3256e8311a8ca5c109abc5c623e2f2f4"

SRC_URI += "https://github.com/dazzysakb/sclbl-tutorials/raw/a9634d9fb7a9cbbc854ebcb23d7e207ee0112807/terms/sclbl-beta-terms-02062020.pdf;name=lic"
SRC_URI[lic.sha256sum] = "3bfe312ddb0e78352d38064590bd743c8062c051bbdaef295da57aa77da4f087"

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
