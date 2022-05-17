FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
SRC_URI_append = " \
               file://bootmisc.sh \
               file://initvar.sh \
               file://umountroot \
               file://vars.sh \
"

FILES_${PN} += "/lib/init/vars.sh"
CONFFILES_${PN} += "${sysconfdir}/init.d/umountroot"

rmfiles = " \
        init.d/populate-volatile.sh \
        init.d/read-only-rootfs-hook.sh \
        init.d/save-rtc.sh \
        rc*.d/*populate-volatile.sh \
        rc*.d/*read-only-rootfs-hook.sh \
        rc*.d/*save-rtc.sh \
        default/volatiles \
"

do_install_append () {
    install -m 0755 ${WORKDIR}/initvar.sh ${D}${sysconfdir}/init.d
    update-rc.d -r ${D} -v initvar.sh start 02 S .

    install -d -m 0755 ${D}${sysconfdir}/.run
    install -d -m 0755 ${D}${sysconfdir}/.var

    install -d -m 0755 ${D}/lib/init
    install -m 0755 ${WORKDIR}/vars.sh ${D}/lib/init/vars.sh
    for f in ${rmfiles};do
        rm -rv ${D}${sysconfdir}/${f}
    done

    install -m 0755 ${WORKDIR}/umountroot       ${D}${sysconfdir}/init.d/umountroot
    update-rc.d -r ${D} -v umountroot start 41 0 6 .

    install -m 0755 ${WORKDIR}/checkfs.sh	${D}${sysconfdir}/init.d/checkfs.sh
    update-rc.d -r ${D} -v checkfs.sh start 04 S .

    update-rc.d -r ${D} -v -f checkroot.sh remove
    update-rc.d -r ${D} -v -f mountall.sh remove

    update-rc.d -r ${D} -v checkroot.sh start 03 S .
    update-rc.d -r ${D} -v mountall.sh start 05 S .
}
