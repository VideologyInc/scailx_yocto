DESCRIPTION = "Your Personal Cloud"
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
    ${CASA_DOWNLOAD_DOMAIN}IceWhaleTech/CasaOS-Gateway/releases/download/v0.4.8-alpha2/linux-${CASAOS_ARCH}-casaos-gateway-v0.4.8-alpha2.tar.gz;name=gateway \
    ${CASA_DOWNLOAD_DOMAIN}IceWhaleTech/CasaOS-MessageBus/releases/download/v0.4.4-3-alpha2/linux-${CASAOS_ARCH}-casaos-message-bus-v0.4.4-3-alpha2.tar.gz;name=messagebus \
    ${CASA_DOWNLOAD_DOMAIN}IceWhaleTech/CasaOS-UserService/releases/download/v0.4.8/linux-${CASAOS_ARCH}-casaos-user-service-v0.4.8.tar.gz;name=userservice \
    ${CASA_DOWNLOAD_DOMAIN}IceWhaleTech/CasaOS-LocalStorage/releases/download/v0.4.4/linux-${CASAOS_ARCH}-casaos-local-storage-v0.4.4.tar.gz;name=localstorage \
    ${CASA_DOWNLOAD_DOMAIN}IceWhaleTech/CasaOS-AppManagement/releases/download/v0.4.8/linux-${CASAOS_ARCH}-casaos-app-management-v0.4.8.tar.gz;name=appmanagement \
    ${CASA_DOWNLOAD_DOMAIN}IceWhaleTech/CasaOS/releases/download/v0.4.8/linux-${CASAOS_ARCH}-casaos-v0.4.8.tar.gz;name=main \
    ${CASA_DOWNLOAD_DOMAIN}IceWhaleTech/CasaOS-CLI/releases/download/v0.4.4-3-alpha1/linux-${CASAOS_ARCH}-casaos-cli-v0.4.4-3-alpha1.tar.gz;name=cli \
"
SRC_URI[gateway.sha256sum] = "ec1ae1ea58def09280ee506d441ddec7b608192fb96e71f8f4b05bd2eb61ee7b"
SRC_URI[messagebus.sha256sum] = "9ea763674a21340c874397aa81fa5fde7e4d5ada574b14fe88871956fa066ecf"
SRC_URI[userservice.sha256sum] = "5396e92f74398801915f446c179cc18b949e0be0a3667bbb104ac9f2c650dfca"
SRC_URI[localstorage.sha256sum] = "b023987bf24251ede5c7d57bdbbe4b8f6ed12cfdbe1ad9e7704bb8f15ac382d2"
SRC_URI[appmanagement.sha256sum] = "0223bc3b969329a30aec882f979030e7f94f3e8125dd76bd3ff66973cf878417"
SRC_URI[main.sha256sum] = "8782dba3d2ece9bfbba071d7a3be8a92eb9ef6af87293e77684d5c547ebc438f"
SRC_URI[cli.sha256sum] = "30f05faa9607c59942d149d31fac15b7c13a19f2b785d937605f0fdc246a47fb"

do_install(){
    cp -rf ${S}/sysroot/etc ${D}${sysconfdir}
    install -d ${D}${bindir}
    cp -rf ${S}/sysroot/usr/bin/* ${D}/${bindir}
    install -d ${D}${systemd_system_unitdir}
    cp -rf ${S}/sysroot/usr/lib/systemd/system/* ${D}/${systemd_system_unitdir}
    rm -f ${D}/${bindir}/appfile2compose

    mv -f ${D}${systemd_system_unitdir}/casaos-app-management.service.buildroot ${D}${systemd_system_unitdir}/casaos-app-management.service
    rm -f ${D}${systemd_system_unitdir}/casaos-gateway.service.buildroot
}

inherit systemd
SYSTEMD_SERVICE:${PN} = " \
    casaos-app-management.service \
    casaos-gateway.service \
    casaos-local-storage.service \
    casaos-message-bus.service \
    casaos.service \
    casaos-user-service.service \
    rclone.service \
"

RDEPENDS:${PN} = "casaos-ui casaos-appstore rclone smartmontools"

FILES_${PN} += "${sysconfdir}/casaos ${bindir} ${systemd_system_unitdir} ${datadir}/casaos ${datadir}/bash_completion.d"

INSANE_SKIP:${PN} += "already-stripped"

RM_WORK_EXCLUDE += "${PN}"

# INSANE_SKIP:${PN} += "file-rdeps"
