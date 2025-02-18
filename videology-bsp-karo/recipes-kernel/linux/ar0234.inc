LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

SRC_URI = "git://github.com/VideologyInc/kernel_mod_vid_ap1302.git;branch=main;protocol=https"
SRCREV ="9058f2f1e6bd1a5e307013c41b871533e42daa38"
S = "${WORKDIR}/git"

PV = "0.1.29"
# RM_WORK_EXCLUDE += "${PN}"
