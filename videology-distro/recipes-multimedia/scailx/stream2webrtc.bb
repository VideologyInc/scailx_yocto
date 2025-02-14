DESCRIPTION = "a python script to monitor a tmp directlry and create go2rtc streams when a unixfd stream is created there."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI += "file://stream2webrtc.py"

PV = "1.0.0"

inherit allarch systemd

SRC_URI += "file://stream2webrtc.service "
S = "${WORKDIR}"

SYSTEMD_SERVICE:${PN} = "stream2webrtc.service"
SYSTEMD_AUTO_ENABLE = "enable"
do_install() {
   	install -D -m0644 "${WORKDIR}/stream2webrtc.service" "${D}${systemd_system_unitdir}/stream2webrtc.service"
	install -D -m0755 "${WORKDIR}/stream2webrtc.py" "${D}${bindir}/stream2webrtc.py"
}

FILES:${PN} = "${systemd_system_unitdir}/ ${bindir}"
