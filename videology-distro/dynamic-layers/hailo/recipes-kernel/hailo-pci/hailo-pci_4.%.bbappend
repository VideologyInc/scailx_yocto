FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://hailo_pci.conf"

do_install:append() {
    install -d ${D}${sysconfdir}/modprobe.d
    install -m 0644 ${WORKDIR}/hailo_pci.conf ${D}${sysconfdir}/modprobe.d/hailo_pci.conf
}

FILES:${PN} += "${sysconfdir}/modprobe.d/"