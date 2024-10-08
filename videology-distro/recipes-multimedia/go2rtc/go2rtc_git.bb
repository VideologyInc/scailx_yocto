DESCRIPTION = "Ultimate camera streaming application with support RTSP, RTMP, HTTP-FLV, WebRTC, MSE, HLS, MP4, MJPEG, HomeKit, FFmpeg, etc."
HOMEPAGE = "https://github.com/AlexxIT/go2rtc"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRCNAME = "go2rtc"

SRCREV = "a4885c2c3abce58074d04878bba0d72105642a9b"
GO_IMPORT = "github.com/AlexxIT/go2rtc"
SRC_URI = "git://${GO_IMPORT};branch=master;protocol=https"
SRC_URI += "file://0001-remove-non-scailx-inputs.patch;patchdir=src/${GO_IMPORT}"

PV = "1.9.4"
GO_EXTRA_LDFLAGS = "-s -w"
export CGO_ENABLED = "0"
GO_INSTALL = "${GO_IMPORT}"
do_compile[network] = "1"

# build executable instead of shared object
GO_LINKSHARED = ""
# GOBUILDFLAGS:remove = "-buildmode=pie"

inherit go-mod

FILES:${PN} += "${bindir}/go2rtc"

# RM_WORK_EXCLUDE += "${PN}"

SRC_URI += "file://go2rtc.yaml"
SRC_URI += "file://go2rtc.service"
SRC_URI += "file://go2rtc-create-cams-config.py"

inherit systemd

SYSTEMD_SERVICE:${PN} = "go2rtc.service"

do_install:append(){
    install -d  ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/go2rtc.service ${D}${systemd_system_unitdir}

    install -d ${D}${sysconfdir}/default
    install -m 0644 ${WORKDIR}/go2rtc.yaml ${D}${sysconfdir}/default/

    install -m 0755 ${WORKDIR}/go2rtc-create-cams-config.py ${D}${bindir}/
    rm -rf ${D}${bindir}/go2rtc_*
}

RDEPENDS:${PN} += "python3-core python3-pyyaml"
FILES:${PN} += "${bindir}/go2rtc ${systemd_system_unitdir}/system ${confdir}"
INSANE_SKIP:${PN} += "already-stripped"
INHIBIT_PACKAGE_STRIP = "1"


