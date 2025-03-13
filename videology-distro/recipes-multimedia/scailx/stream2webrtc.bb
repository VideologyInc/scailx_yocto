DESCRIPTION = "a python script to monitor a tmp directlry and create go2rtc streams when a unixfd stream is created there."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI += "file://stream2webrtc.py"

PV = "1.0.0"

inherit allarch systemd

SRC_URI += "file://scailxwebsink.py"
S = "${WORKDIR}"

do_install() {
	install -D -m0755 "${WORKDIR}/scailxwebsink.py" "${D}${libdir}/gstreamer-1.0/python/scailxwebsink.py"
}

FILES:${PN} = "${libdir}/gstreamer-1.0/python/"

RDEPENDS:${PN} = "python3-watchdog python3-requests python3-termcolor gstreamer1.0-plugins-bad"