LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

SRC_URI = "git://github.com/VideologyInc/kernel-module-crosslink.git;branch=master;protocol=https"
# Latest update of AB#2139 with boson_stats.py etc.
# SRCREV = "cf2e5e5d3138b3ff166533569cb2abcecfa4e011"
# Latest update of AB#2248 plus AB#2249 with go2rtc fix.
SRCREV = "b8319bb822d54e7e4e3a8a2aec5c9aeec5762590"

S = "${WORKDIR}/git"

PV = "0.13.13"


