LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

SRC_URI = "git://github.com/VideologyInc/kernel_mod_vid_ap1302.git;branch=main;protocol=https"
SRCREV ="bcf1b69bab26d16449c29f1ed2485f74024b804f"
S = "${WORKDIR}/git"

RM_WORK_EXCLUDE += "${PN}"
