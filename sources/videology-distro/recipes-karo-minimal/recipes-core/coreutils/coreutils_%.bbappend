base_bindir_progs = "date"
bindir_progs = ""
sbindir_progs = ""

ALTERNATIVE_PRIORITY = "0"
ALTERNATIVE_PRIORITY[date] = "300"
ALTERNATIVE_${PN} = "date"
ALTERNATIVE_${PN}-doc = ""

do_install_append () {
    echo "base_bindir = ${base_bindir}"
    echo "bindir      = ${bindir}"
    echo "sbindir     = ${sbindir}"
    echo "datarootdir = ${datarootdir}"
    echo "sharedstatedir = ${sharedstatedir}"

    rm -rvf "${D}${bindir}"
    # remove now empty directories
    echo "removing empty directories"
    find ${D}${base_bindir} ${D}${sbindir} -depth -type d -empty -exec rmdir -v \{\} \;
}
