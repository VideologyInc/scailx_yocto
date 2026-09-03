LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

SRC_URI = "git://github.com/VideologyInc/kernel-module-crosslink.git;branch=master;protocol=https"

# Latest update of AB#2418 B8 lvds2mipi driver.
# SRCREV = "f33e2364637ecb16d4c11fcea8ef83c6231d8b76"

# Latest ZoomBlock settings and dual-ZoomBlock fixes for go2RTC.
SRCREV = "45840d63f9e1d46aae32e6a1cf1b983f009171d4"

S = "${WORKDIR}/git"

PV = "0.20.10"

