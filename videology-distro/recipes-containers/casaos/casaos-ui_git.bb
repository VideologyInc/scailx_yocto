DESCRIPTION = "CasaOS-CLI"
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
    ${CASA_DOWNLOAD_DOMAIN}IceWhaleTech/CasaOS-UI/releases/download/v0.4.8/linux-all-casaos-v0.4.8.tar.gz;name=ui \
"
SRC_URI[ui.sha256sum] = "f3d94cdf654099ce7d52182548509623f47a346e83fd1ab0903a5d0672978069"

do_install(){
    cp -rf ${S}/sysroot/etc ${D}${sysconfdir}
    cp -rf ${S}/sysroot/var ${D}${localstatedir}
}

RDEPENDS:${PN} = "bash"

# RM_WORK_EXCLUDE += "${PN}"

INSANE_SKIP:${PN} += "file-rdeps"
