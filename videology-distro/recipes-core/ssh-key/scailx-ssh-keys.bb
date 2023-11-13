DESCRIPTION = "Scailx default ssh-keys"

PR = "r0"

LICENSE = "CLOSED"

SRC_URI =  "file://scailx-dev.pub"
SRC_URI += "file://scailx-keys.sh"

S = "${WORKDIR}"

RDEPENDS:${PN} = "openssh bash"

do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install() {
    install -d ${D}/${sysconfdir}/ssh
    install -m 0600 ${S}/scailx-dev.pub ${D}/${sysconfdir}/ssh/authorized_keys_scailx_dev
    install -m 0700 ${S}/scailx-keys.sh ${D}/${sysconfdir}/ssh/scailx-keys.sh
    touch ${D}/${sysconfdir}/ssh/authorized_keys
}

FILES_${PN} += "/${sysconfdir}/ssh"
