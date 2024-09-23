
do_add_scailx_ppa() {
    [ -f ${IMAGE_ROOTFS}${sysconfdir}/apt/sources.list.d/${DISTRO}.list ] && rm -f ${IMAGE_ROOTFS}${sysconfdir}/apt/sources.list.d/${DISTRO}.list
    for f in ${PACKAGE_ARCHS}; do
        [ -d ${WORKDIR}/oe-rootfs-repo/$f ] || continue
        echo "deb [signed-by=/etc/pki/scailx-ppa-key.pub.gpg] https://scailx-ppa.org/${DISTRO}/${DISTRO_VERSION}/deploy/deb/$f ./" >> ${IMAGE_ROOTFS}${sysconfdir}/apt/sources.list.d/${DISTRO}.list
    done
}
IMAGE_PREPROCESS_COMMAND += '${@bb.utils.contains("IMAGE_FEATURES", "package-management", "do_add_scailx_ppa", "", d)}'
