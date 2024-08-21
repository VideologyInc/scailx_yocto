LICENSE = "CLOSED"
SECTION = "multimedia"

SRC_URI = "https://scailx-ppa.org/scailx-apps/scailx-ai-portal/scailx-ai-portal_24.08.2.zip;subdir=git;name=zip"
SRC_URI += "file://scailx-ai-portal.service"
SRC_URI[zip.sha256sum] = "9cf59783196b91b6d73390b4bad9b422ba1f549eb155ae2d1d6e82434e73722e"

# Modify these as desired
PV = "24.08.1"
S = "${WORKDIR}/git"

RDEPENDS:${PN} += "nodejs python3-core scailx-gst-plugins python3-websockets"

# RM_WORK_EXCLUDE += "${PN}"

do_install(){
    install -d ${D}${systemd_system_unitdir}
    cp -r ${S}/* ${D}
    install -m 0644 ${WORKDIR}/scailx-ai-portal.service ${D}${systemd_system_unitdir}
}

FILES:${PN} += "${prefix}/${PN}"
inherit systemd
SYSTEMD_SERVICE:${PN} = "scailx-ai-portal.service"
SYSTEMD_AUTO_ENABLE = "disable"
INSANE_SKIP:${PN} += "installed-vs-shipped"