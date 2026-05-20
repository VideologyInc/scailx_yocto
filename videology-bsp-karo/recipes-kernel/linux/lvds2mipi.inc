LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

SRC_URI = "git://github.com/VideologyInc/kernel-module-crosslink.git;branch=master;protocol=https"

# Latest update of AB#2354 to add camera id and device path to go2RTC and Portal.
SRCREV = "222d0411174f28a79cd5eaf716f88e2b60a72877"

S = "${WORKDIR}/git"

PV = "0.13.19"


