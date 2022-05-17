inherit relative_symlinks useradd

PACKAGECONFIG_remove_karo-minimal = "openssl"

remove_ntp_conf = "${@ bb.utils.contains('DISTRO_FEATURES', 'dhcpcd', True, False, d) and \
                       bb.utils.contains('IMAGE_FEATURES', 'read-only-rootfs', True, False, d)}"

FILES_${PN}_remove := "${@ "${sysconfdir}/ntp.conf" if ${remove_ntp_conf} else ""}"

DEPENDS += "busybox"

USERADD_PACKAGES += "${PN}"
USERADD_PARAM_${PN} = "--system -U -d /nonexistent --no-create-home -s /bin/false ntp"
GROUPADD_PARAM_${PN} = "-f --system crontab"

CRONTABS_DIR = "${localstatedir}/spool/cron/crontabs"
FILES_${PN} += "${CRONTABS_DIR}/root"

do_install_append() {
    if [ "${remove_ntp_conf}" = "True" ];then
        rm -f ${D}${sysconfdir}/ntp.conf
    fi
}

pkg_postinst_ntpdate() {
    if ! grep -q -s ntpdate $D${CRONTABS_DIR}/root; then
        echo "adding crontab"
        test -d $D${CRONTABS_DIR} || install -d -g crontab -m 1730 $D${CRONTABS_DIR}
        echo "30 * * * *    ${bindir}/ntpdate-sync silent" >> $D${CRONTABS_DIR}/root
    fi
# without this line the script will fail with: "ERROR: ntp: useradd command did not succeed"
}
