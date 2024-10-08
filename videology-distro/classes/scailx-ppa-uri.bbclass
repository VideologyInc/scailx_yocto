python () {
    if 'package-management' in d.getVar('IMAGE_FEATURES').split():
        if 'package_deb' in d.getVar('PACKAGE_CLASSES').split():
            d.appendVar('IMAGE_PREPROCESS_COMMAND', ' do_add_scailx_ppa ')
}

do_add_scailx_ppa() {
    [ -f ${IMAGE_ROOTFS}${sysconfdir}/apt/sources.list.d/${DISTRO}.list ] && rm -f ${IMAGE_ROOTFS}${sysconfdir}/apt/sources.list.d/${DISTRO}.list
    for f in ${PACKAGE_ARCHS}; do
        [ -d ${WORKDIR}/oe-rootfs-repo/$f ] || continue
        echo "deb [signed-by=/etc/pki/scailx-ppa-key.pub.gpg] https://scailx-ppa.org/${DISTRO}/${DISTRO_VERSION}/deploy/deb/$f ./" >> ${IMAGE_ROOTFS}${sysconfdir}/apt/sources.list.d/${DISTRO}.list
    done
}
