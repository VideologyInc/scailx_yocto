SUMMARY = "HTTP server which provides buttons to send visca commands to a serial port."
LICENSE = "CLOSED"

SRC_URI = " \
        file://visca-control.py \
        file://visca-control.service \
"

S = "${WORKDIR}"

# inherit setuptools3
inherit systemd

RDEPENDS:${PN} += "python3-core"
SYSTEMD_SERVICE:${PN} = "visca-control.service"

do_install() {
    install -d "${D}${bindir}"
    install -m 0755 visca-control.py "${D}${bindir}"

    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 visca-control.service ${D}${systemd_system_unitdir}
    fi
}
