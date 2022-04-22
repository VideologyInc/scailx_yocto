DESCRIPTION = "Modprobe config to load basler-camera driver before imx8-media-dev driver."
LICENSE = "CLOSED"

SRC_URI = "file://imx8-media-dev.conf"

S = "${WORKDIR}"


do_install() {
	install -d ${D}/etc/modprobe.d/
	install -m 0755 imx8-media-dev.conf ${D}/etc/modprobe.d/
}

FILES_${PN} += "/etc/modprobe.d"

