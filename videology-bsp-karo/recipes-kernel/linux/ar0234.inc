LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://${WORKDIR}/git/LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

SRC_URI = "git://github.com/VideologyInc/kernel_mod_vid_ap1302.git;branch=main;protocol=https"
SRCREV ="3401b72d1b444583f9ee7ca344c6cbac64e5d7fb"

# SRCREV ="f40e2be34b45f526c1ebfdb2b01d9bacb789572a"

S = "${WORKDIR}/git"

PV = "0.1.30"
# RM_WORK_EXCLUDE += "${PN}"
