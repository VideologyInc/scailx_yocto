DESCRIPTION = "Ultimate camera streaming application with support RTSP, RTMP, HTTP-FLV, WebRTC, MSE, HLS, MP4, MJPEG, HomeKit, FFmpeg, etc."
HOMEPAGE = "https://github.com/AlexxIT/go2rtc"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRCNAME = "go2rtc"

SRCREV = "ccec41a10f2636676befea50aa4cee2aa9d1d75a"
GO_IMPORT = "github.com/AlexxIT/go2rtc"
SRC_URI = "git://${GO_IMPORT};branch=master;protocol=https"

GO_EXTRA_LDFLAGS = "-s -w"
export CGO_ENABLED = "0"
# GO_INSTALL = "${GO_IMPORT}"
do_compile[network] = "1"

# build executable instead of shared object
GO_LINKSHARED = ""
# GOBUILDFLAGS:remove = "-buildmode=pie"

inherit go-mod

FILES:${PN} += "${bindir}/go2rtc"

RM_WORK_EXCLUDE += "${PN}"

# SRCBIN = "go2rtc_linux_amd64"
# SRCBIN:aarch64 = "go2rtc_linux_arm64"
# SRCBIN:arm = "go2rtc_linux_arm"

# SRC_URI = "https://${GO_IMPORT}/releases/download/v1.8.5/${SRCBIN};name=go2rtc-${HOST_ARCH}"
# SRC_URI[go2rtc-aarch64.sha256sum] = "1fa197780b0889277f34e6ee510b32cdf7ddfe60ad1e724308d6748fcdf7c53c"
# SRC_URI[go2rtc-x86_64.sha256sum] = "a10b03ee68e4502933eaf2c1cda871a9e5b1080fd775a85115ee6c15a5fcde1d"

# COMPATIBLE_HOST = "(aarch64|x86_64).*-linux"

SRC_URI += "file://go2rtc.yaml"
SRC_URI += "file://go2rtc.service"

inherit systemd

SYSTEMD_SERVICE:${PN} = "go2rtc.service"

do_install:append(){
    install -d  ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/go2rtc.service ${D}${systemd_system_unitdir}

#     install -d ${D}${bindir}
#     for f in ${WORKDIR}/go2rtc_*; do install -m 0755 $f ${D}${bindir}; ln -s -T $f ${D}${bindir}/go2rtc; done
#     # install -m 0755 ${WORKDIR}/go2rtc* ${D}${bindir}
#     # ln -s ${bindir}/go2rtc* ${D}${bindir}/go2rtc

    install -d ${D}${sysconfdir}/default
    install -m 0644 ${WORKDIR}/go2rtc.yaml ${D}${sysconfdir}/default/
}

FILES:${PN} += "${bindir}/go2rtc ${systemd_system_unitdir}/system ${confdir}"
INSANE_SKIP:${PN} += "already-stripped"
INHIBIT_PACKAGE_STRIP = "1"







