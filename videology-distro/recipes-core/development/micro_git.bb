SUMMARY = "Modern and intuitive terminal-based text editor"
HOMEPAGE = "https://github.com/zyedidia/micro"
DESCRIPTION = "A modern and intuitive terminal-based text editor written in Go, \
               featuring mouse support, syntax highlighting, and plugin system."

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://src/${GO_IMPORT}/LICENSE;md5=5d3b6c44af6e9177296f05709aab74a4"
inherit go

GO_IMPORT = "github.com/zyedidia/micro"

SRC_URI = "git://${GO_IMPORT};protocol=https;branch=master"
SRCREV = "04c577049ca898f097cd6a2dae69af0b4d4493e1"
PV = "2.0.14"

S = "${WORKDIR}/git"
GO_WORKDIR ?= "${GO_IMPORT}"

# GO_EXTRA_LDFLAGS = "-s -w"
# export CGO_ENABLED = "0"
do_compile[network] = "1"

inherit go

do_compile() {
	# Pass the needed cflags/ldflags so that cgo
	# can find the needed headers files and libraries
	export GOARCH=${TARGET_GOARCH}
	# export CGO_ENABLED="1"

	cd ${S}/src/${GO_IMPORT}
	oe_runmake build
}

# INSANE_SKIP:${PN} += "already-stripped"
RM_WORK_EXCLUDE += "${PN}"
