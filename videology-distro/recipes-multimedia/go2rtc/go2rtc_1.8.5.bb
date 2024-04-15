DESCRIPTION = "Ultimate camera streaming application with support RTSP, RTMP, HTTP-FLV, WebRTC, MSE, HLS, MP4, MJPEG, HomeKit, FFmpeg, etc."
HOMEPAGE = "https://github.com/AlexxIT/go2rtc"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRCNAME = "go2rtc"

SRCREV = "ccec41a10f2636676befea50aa4cee2aa9d1d75a"
GO_IMPORT = "github.com/AlexxIT/go2rtc"
SRC_URI = "git://${GO_IMPORT};branch=master;protocol=https"

inherit go-mod systemd

# S = "${WORKDIR}/git"

# do_compile:prepend() {
# 	export GO111MODULE=off
# }


GO_INSTALL = "${GO_IMPORT}"
do_compile[network] = "1"

# build executable instead of shared object
GO_LINKSHARED = ""
GOBUILDFLAGS:remove = "-buildmode=pie"


# FILES:${PN} += "/usr/local/bin"