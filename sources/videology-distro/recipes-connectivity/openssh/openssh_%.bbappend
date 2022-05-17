inherit relative_symlinks

FILES_${PN}_append = "/run/ /run/sshd ${localstatedir}/log ${localstatedir}/log/lastlog"

do_install_append() {
    install -v -d -m 0755 ${D}/run
    install -v -d -m 0755 ${D}/run/sshd
    install -v -d -m 0755 ${D}${localstatedir}/log/
    install -v -m 0644 /dev/null ${D}${localstatedir}/log/lastlog
    rm -rvf ${D}${sysconfdir}/default/volatiles
}

RRECOMMENDS_${PN}-sshd_remove_class-target = " rng-tools"
RRECOMMENDS_${PN}-sshd_append_class-target = " haveged"
