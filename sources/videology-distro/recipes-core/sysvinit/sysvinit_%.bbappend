FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
inherit relative_symlinks

do_install_append () {
    rm -rvf ${D}${sysconfdir}/default/volatiles

    if ${@ bb.utils.contains('MACHINE_FEATURES', 'emmc', 'true', 'false', d)} && \
            ${@bb.utils.contains('VIRTUAL-RUNTIME_init_manager','sysvinit','true','false',d)}; then
        # Change the value of ENABLE_ROOTFS_FSCK in ${sysconfdir}/default/rcS to yes
        sed -i '/^ENABLE_ROOTFS_FSCK=/s/no/yes/' ${D}${sysconfdir}/default/rcS
    fi
}
