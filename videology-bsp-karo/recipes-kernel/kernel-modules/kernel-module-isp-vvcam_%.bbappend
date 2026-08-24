FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-media-vvcam-fix-OS08A20-format-and-mode-handling.patch;patchdir=../.."
SRC_URI += "file://ox05b1s_mipi.conf"

do_install:append() {
    install -d ${D}${sysconfdir}/modprobe.d
    install -m 0644 ${WORKDIR}/ox05b1s_mipi.conf ${D}${sysconfdir}/modprobe.d/ox05b1s_mipi.conf
}

