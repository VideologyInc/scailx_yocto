DESCRIPTION = "A small python utility for controlling video4linux cameras. It queries user-controls from the v4l2 devices and creates a TUI to display and adjust their values"
HOMEPAGE = "https://github.com/antmicro/pyvidctrl"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"

SRC_URI = "git://github.com/antmicro/pyvidctrl.git;branch=master;protocol=https"
SRCREV ="865d76b3b686c22a74e02b75b931d18138e81638"
S = "${WORKDIR}/git"

inherit setuptools3

RDEPENDS:${PN} += "python3-v4l2"
