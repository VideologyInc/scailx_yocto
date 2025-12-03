LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://${WORKDIR}/git/LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

# SRC_URI = "git://github.com/VideologyInc/kernel_mod_vid_ap1302.git;branch=main;protocol=https"
# SRCREV ="d5d2eb623b356b5fc710c036c9c3f495a8fc6719"

SRC_URI = "git://github.com/VideologyInc/kernel_mod_vid_ap1302.git;branch=AB#1803-fix-brightness;protocol=https"
SRCREV ="b9c97b9f04fc9d836273f25f54f1818f455ef21a"
S = "${WORKDIR}/git"

PV = "0.1.30"
# RM_WORK_EXCLUDE += "${PN}"
