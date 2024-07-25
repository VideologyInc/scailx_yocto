DESCRIPTION = "CasaOS-AppManagement"
SECTION = "casa-os"
HOMEPAGE = "https://https://casaos.io/"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

CASA_DOWNLOAD_DOMAIN = "https://github.com/"

CASAOS_ARCH ?= "${MACHINE_SOCARCH}"
CASAOS_ARCH:armv8a = "arm64"
CASAOS_ARCH:armv7a = "arm-7"
CASAOS_ARCH:x86_64 = "amd64"

S = "${WORKDIR}/build"
SRC_URI= " \
    ${CASA_DOWNLOAD_DOMAIN}IceWhaleTech/CasaOS-AppStore/releases/download/v0.4.5/linux-all-appstore-v0.4.5.tar.gz;name=appstore \
"
SRC_URI[appstore.sha256sum] = "4f346a057097c09706813052c3ef5d0ef49b609aea320b4fcf3af7609b3957b5"

do_install(){
    cp -rf ${S}/sysroot/var ${D}${localstatedir}
}

# RDEPENDS:${PN} = "bash"

RM_WORK_EXCLUDE += "${PN}"

INSANE_SKIP:${PN} += "file-rdeps"
