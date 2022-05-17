inherit relative_symlinks

FILES_${PN}_remove = "${sysconfdir}/default/volatiles/volatiles.04_pulse"

do_install_append() {
    rm -rvf ${D}${sysconfdir}/default/volatiles
}
