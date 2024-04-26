DESCRIPTION = "Ultimate camera streaming application with support RTSP, RTMP, HTTP-FLV, WebRTC, MSE, HLS, MP4, MJPEG, HomeKit, FFmpeg, etc."
HOMEPAGE = "https://github.com/AlexxIT/go2rtc"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRCNAME = "go2rtc"

# SRCREV = "ccec41a10f2636676befea50aa4cee2aa9d1d75a"
GO_IMPORT = "github.com/AlexxIT/go2rtc"
# SRC_URI = "git://${GO_IMPORT};branch=master;protocol=https"

# inherit go-mod systemd

# # S = "${WORKDIR}/git"

# # do_compile:prepend() {
# # 	export GO111MODULE=off
# # }


# GO_INSTALL = "${GO_IMPORT}"
# do_compile[network] = "1"

# # build executable instead of shared object
# GO_LINKSHARED = ""
# GOBUILDFLAGS:remove = "-buildmode=pie"


# # FILES:${PN} += "/usr/local/bin"

SRCBIN = "go2rtc_linux_amd64"
SRCBIN:aarch64 = "go2rtc_linux_arm64"
SRCBIN:arm = "go2rtc_linux_arm"

SRC_URI = "https://${GO_IMPORT}/releases/download/v1.8.5/${SRCBIN};name=go2rtc-${HOST_ARCH}"
SRC_URI[go2rtc-aarch64.sha256sum] = "1fa197780b0889277f34e6ee510b32cdf7ddfe60ad1e724308d6748fcdf7c53c"
SRC_URI[go2rtc-x86_64.sha256sum] = "a10b03ee68e4502933eaf2c1cda871a9e5b1080fd775a85115ee6c15a5fcde1d"

COMPATIBLE_HOST = "(aarch64|x86_64).*-linux"

SRC_URI += "file://go2rtc.yaml"
SRC_URI += "file://go2rtc.service"

inherit systemd
S = "${WORKDIR}/git"

SYSTEMD_SERVICE:${PN} = "go2rtc.service"

do_install:append(){
    install -d  ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/go2rtc.service ${D}${systemd_system_unitdir}

    install -d ${D}${bindir}
    for f in ${WORKDIR}/go2rtc*; do install -m 0755 $f ${D}${bindir}; done
    # install -m 0755 ${WORKDIR}/go2rtc* ${D}${bindir}
    # ln -s ${bindir}/go2rtc* ${D}${bindir}/go2rtc

    install -d ${D}${sysconfdir}/default
    install -m 0644 ${WORKDIR}/go2rtc.yaml ${D}${sysconfdir}/default/
}

FILES:${PN} += "${bindir} ${systemd_system_unitdir}/system ${confdir}"
INSANE_SKIP:${PN} += "already-stripped"

