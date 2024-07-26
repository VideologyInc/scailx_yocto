LICENSE = "CLOSED"
SECTION = "multimedia"

SRC_URI = "file://scailx-ai-portal.zip;subdir=git"
SRC_URI += "file://scailx-ai-portal.service"

# Modify these as desired
PV = "0.1.0"
# SRCREV = "e6c2cf7e5ed3f32593fc0efcb4b8e7bbebc77e34"
S = "${WORKDIR}/git"

RDEPENDS:${PN} += "nodejs python3-core scailx-gst-plugins python3-websockets"

RM_WORK_EXCLUDE += "${PN}"

do_install(){
    install -d ${D}${prefix}/${PN}
    install -d ${D}${bindir}
    install -d ${D}${systemd_system_unitdir}
    cp -r ${S}/* ${D}${prefix}/${PN}
    mv ${D}${prefix}/${PN}/signaling_server.py ${D}${bindir}/
    install -m 0644 ${WORKDIR}/scailx-ai-portal.service ${D}${systemd_system_unitdir}
}

FILES:${PN} += "${prefix}/${PN}"
inherit systemd
SYSTEMD_SERVICE:${PN} = "scailx-ai-portal.service"
SYSTEMD_AUTO_ENABLE = "disable"