FILES_${PN} += "/run/dbus"

do_install_append_class-target() {
    rm -rvf ${D}${sysconfdir}/default/volatiles
    install -v -d -o messagebus -g messagebus -m 0755 ${D}/run/dbus
}
