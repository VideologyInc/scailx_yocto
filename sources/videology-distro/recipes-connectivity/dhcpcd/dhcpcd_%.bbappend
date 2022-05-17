FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

PACKAGECONFIG_remove = "udev"
FILES_${PN} += " \
            /run/dhcpcd/hook-state \
            ${localstatedir}/lib/dhcpcd \
            ${sysconfdir}/ntp.conf \
"

SRC_URI_append = " \
           file://ntp.conf \
           file://resolv.conf \
"

inherit relative_symlinks

do_install_append () {
    sed -i 's/^duid/#duid/;s/^#clientid/clientid/' ${D}${sysconfdir}/dhcpcd.conf
    if ! grep -q '^quiet' ${D}${sysconfdir}/dhcpcd.conf;then
        cat << EOF >> ${D}${sysconfdir}/dhcpcd.conf

# Silence debug messages
quiet
EOF
    fi
    if ${@ bb.utils.contains('IMAGE_FEATURES','read-only-rootfs','true','false',d)};then
        install -v -d -m 0755 ${D}/run/dhcpcd/hook-state

        install -D -m 0644 ${WORKDIR}/ntp.conf ${D}/run/dhcpcd/hook-state/ntp.conf/eth0.dhcp
        ln -snvf /run/dhcpcd/hook-state/ntp.conf/eth0.dhcp ${D}${sysconfdir}/ntp.conf

        install -D -m 0644 ${WORKDIR}/resolv.conf ${D}/run/dhcpcd/hook-state/resolv.conf/eth0.dhcp
        ln -snvf /run/dhcpcd/hook-state/resolv.conf/eth0.dhcp ${D}${sysconfdir}/resolv.conf

        install -v -m 0755 -d ${D}${localstatedir}/lib/dhcpcd
        install -v -m 0755 /dev/null ${D}${localstatedir}/lib/dhcpcd/dhcpcd.duid
        ln -snvf ${localstatedir}/lib/dhcpcd/dhcpcd.duid ${D}${sysconfdir}/
    fi
}
