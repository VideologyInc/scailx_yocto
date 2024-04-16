DESCRIPTION = "python v4l bindings"
LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://LICENSE;md5=751419260aa954499f7abaabaa882bbe"

SRC_URI = "git://github.com/antmicro/python3-v4l2.git;branch=master;protocol=https"
SRCREV ="e56ee8574b6dc15f305f23fb872f2038eccf715a"
S = "${WORKDIR}/git"

inherit setuptools3
