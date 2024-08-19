DESCRIPTION = "Scailx default ssh-keys"

PR = "r0"

LICENSE = "CLOSED"

SRC_URI += "file://overlay_backup.sh"
SRC_URI += "file://session-limits.sh"

S = "${WORKDIR}"

RDEPENDS:${PN} = "bash"

do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install() {
    install -d ${D}${sysconfdir}/profile.d/
    install -m 0755 ${S}/overlay_backup.sh "${D}${sysconfdir}/profile.d"
    install -m 0755 ${S}/session-limits.sh "${D}${sysconfdir}/profile.d"
}

FILES:${PN} = "${sysconfdir}/profile.d"
