SUMMARY = "MediaMTX / rtsp-simple-server is a ready-to-use and zero-dependency server and proxy that allows users to publish, read and proxy live video and audio streams."
GO_IMPORT = "github.com/bluenviron/mediamtx"
HOMEPAGE = "https://github.com/bluenviron/mediamtx"
LICENSE = "MIT"
# LIC_FILES_CHKSUM = "file://src/${GO_IMPORT}/LICENSE;md5=77fd2623bd5398430be5ce60489c2e81"
LIC_FILES_CHKSUM = "file://src/${GO_IMPORT}/LICENSE;md5=0d93a616752fe44d16a3ddbcfb3340be"

SRC_URI = "git://${GO_IMPORT};branch=main;protocol=https"
SRCREV = "b92c8c2613c9cde7dcc431031c2e4ddda1123796"

GO_INSTALL = "${GO_IMPORT}"
do_compile[network] = "1"

# build executable instead of shared object
GO_LINKSHARED = ""
GOBUILDFLAGS:remove = "-buildmode=pie"

inherit go-mod

FILES:${PN} += "/usr/local/bin"
