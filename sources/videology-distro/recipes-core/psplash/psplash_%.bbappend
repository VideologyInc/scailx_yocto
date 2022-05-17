FILES_${PN} += "/mnt/.psplash"

do_install_append() {
    install -d -m 0775 ${D}/mnt/.psplash
}
