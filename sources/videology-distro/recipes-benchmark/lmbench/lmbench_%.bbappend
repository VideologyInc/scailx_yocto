FILES_${PN}_remove = "${sysconfdir}/default/volatiles/99_lmbench"

do_install_append() {
    rm -rvf ${D}${sysconfdir}/default/volatiles
}
