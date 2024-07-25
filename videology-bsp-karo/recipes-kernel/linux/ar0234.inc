LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

SRC_URI = "git://github.com/VideologyInc/kernel_mod_vid_ap1302.git;branch=main;protocol=https"
SRCREV ="4ec17748e893018c54d3d5bdb9d88674c95482a6"
S = "${WORKDIR}/git"

PV = "0.1.29"
RM_WORK_EXCLUDE += "${PN}"
