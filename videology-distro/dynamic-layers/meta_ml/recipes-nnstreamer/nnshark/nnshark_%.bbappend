do_configure[network] = "1"

NNSHARK_SRC = "git://github.com/nxp-imx/nnshark.git;protocol=https"
SRC_URI += "git://gitlab.freedesktop.org/gstreamer/common.git;branch=master;name=common;destsuffix=git/common;protocol=https"
SRCREV_common = "b64f03f6090245624608beb5d2fff335e23a01c0"
SRCREV_FORMAT = "nnshark"