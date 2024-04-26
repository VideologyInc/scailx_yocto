DESCRIPTION = "Kernel loadable module for Videology gs-ar0234"

require ar0234.inc
inherit module

RRECOMMENDS:${PN} += "python3-ar0234"

FILES:${PN} += "${nonarch_base_libdir}/firmware"
FILES:${PN} += "${sysconfdir}/modprobe.d"

do_install:append(){
    install -d ${D}${sysconfdir}/modprobe.d/
    install -m 0644 ${S}/modprobe/* ${D}${sysconfdir}/modprobe.d/
}
