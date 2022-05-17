FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:${THISDIR}/files:${THISDIR}/patches:"

SRC_URI_remove = " \
                 file://ftpget.cfg \
                 file://login-utilities.cfg \
                 file://resize.cfg \
                 file://sha1sum.cfg \
"
SRC_URI_append = " \
                 ${@ bb.utils.contains('DISTRO_FEATURES','pam','file://pam.cfg','',d)} \
                 ${@ bb.utils.contains('DISTRO_FEATURES','busybox-crond','','file://no-crond.cfg',d)} \
"
FILES_${PN} += "${sysconfdir}/network/run"

# overrule prio 200 of sysvinit and shadow
# if enabled, /bin/sh will be linked to /bin/busybox.nosuid
# as workaround this is disabled, and will increase the size of the rootfs
#ALTERNATIVE_PRIORITY = "201"

#BUSYBOX_SPLIT_SUID = "0"

PACKAGES =+ "${PN}-inetd"
FILES_${PN}-inetd = "${sysconfdir}/init.d/${PN}-inetd"
PROVIDES += "${PN}-inetd"

INITSCRIPT_NAME_${PN}-inetd = "${PN}-inetd"
INITSCRIPT_PARAMS_${PN}-inetd = "start 02 2 3 4 5 . stop 01 0 1 6 ."
INITSCRIPT_PACKAGES += "${PN}-inetd"
RDEPENDS_${PN}-inetd = "busybox"
RRECOMMENDS_${PN} += "${INITSCRIPT_PACKAGES}"

inherit useradd relative_symlinks

USERADD_PACKAGES += "${PN}"
USERADD_PARAM_${PN} = ""
GROUPADD_PARAM_${PN} = "--system utmp"

# need /etc/group in the staging directory
DEPENDS += "base-passwd"

FILES_${PN} += "/run/utmp ${localstatedir}/log/wtmp"

do_install_append () {
    if [ -e ${D}${sysconfdir}/init.d/inetd.${BPN} ];then
        mv -vi ${D}${sysconfdir}/init.d/inetd.${BPN} ${D}${sysconfdir}/init.d/${PN}-inetd
    fi

    install -d -m 0755 ${D}${sysconfdir}/network
    ln -snvf /run/network ${D}${sysconfdir}/network/run

    install -v -d -m 0755 ${D}/run
    install -v -m 0664 -g utmp /dev/null ${D}/run/utmp

    install -v -d -m 0755 ${D}${localstatedir}/log
    install -v -m 0664 -g utmp /dev/null ${D}${localstatedir}/log/wtmp

    if ${@ bb.utils.contains('MACHINE_FEATURES', 'emmc', 'true', 'false', d)};then
        if grep -q "CONFIG_INIT=y" ${B}/.config && \
                ${@bb.utils.contains('VIRTUAL-RUNTIME_init_manager','busybox','true','false',d)}; then
            # Change the value of ENABLE_ROOTFS_FSCK in ${sysconfdir}/default/rcS to yes
            sed -i '/^ENABLE_ROOTFS_FSCK=/s/no/yes/' ${D}${sysconfdir}/default/rcS
        fi
    fi
}
