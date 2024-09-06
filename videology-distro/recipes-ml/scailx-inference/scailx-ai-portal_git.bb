LICENSE = "CLOSED"
SECTION = "multimedia"

SRC_URI = "https://scailx-ppa.org/scailx-apps/scailx-ai-portal/scailx-ai-portal_${PV}.zip;subdir=git;name=zip"
SRC_URI += "file://scailx-ai-portal.service"
SRC_URI[zip.sha256sum] = "c8d2b111439d601e83b75ff6ae4b4af028d0c425fd11750d4d2dab69bc6153b9"

# Modify these as desired
PV = "v24.08.4"
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
# SYSTEMD_AUTO_ENABLE = "disable"
INSANE_SKIP:${PN} += "installed-vs-shipped"