LICENSE = "CLOSED"
SECTION = "multimedia"

SRC_URI = "https://scailx-ppa.org/scailx-apps/scailx-ai-portal/scailx-ai-portal_${PV}.zip;subdir=git;name=zip"
SRC_URI += "file://scailx-ai-portal.service"
SRC_URI += "file://pipeline.json"

SRC_URI[zip.sha256sum] = "a65b1d7528cd1c3db82c6ddfc91668e622ab29dbc392325cafbc27055c23e030"

# Modify these as desired
PV = "v24.09.1"
S = "${WORKDIR}/git"

RDEPENDS:${PN} += "nodejs python3-core scailx-gst-plugins python3-websockets"

# RM_WORK_EXCLUDE += "${PN}"

do_install(){
    install -d ${D}${sysconfdir}/scailx-ai-portal/settings
    install -d ${D}${systemd_system_unitdir}
    cp -r ${S}/* ${D}
    install -m 0644 ${WORKDIR}/scailx-ai-portal.service ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/pipeline.json ${D}${sysconfdir}/scailx-ai-portal/settings/pipeline.json
}

FILES:${PN} += "${prefix}/${PN} ${sysconfdir}/scailx-ai-portal ${systemd_system_unitdir}"
inherit systemd
SYSTEMD_SERVICE:${PN} = "scailx-ai-portal.service"
# SYSTEMD_AUTO_ENABLE = "disable"
CONFFILES:${PN} += "${sysconfdir}/scailx-ai-portal/settings/pipeline.json"
INSANE_SKIP:${PN} += "installed-vs-shipped"