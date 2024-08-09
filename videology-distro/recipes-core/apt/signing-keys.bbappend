
pkg_postinst:${PN} () {
    cd $D${sysconfdir}/pki/packagefeed-gpg
    for f in *.gpg; do
        apt-key add $f
        gpg --no-default-keyring --keyring ./pkg_key.gpg --import $f
        gpg --no-default-keyring --keyring ./pkg_key.gpg --export > $D${sysconfdir}/apt/trusted.gpg.d/${f%.gpg}.gpg
    done
}