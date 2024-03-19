LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

SRC_URI = "git://github.com/VideologyInc/scailx_2gs234_v4l.git;branch=main;protocol=https"
SRCREV ="aa86358e1e14bf61acea1de1522634120433a632"
S = "${WORKDIR}/git"

RM_WORK_EXCLUDE += "${PN}"
