PACKAGES_remove = "${PN}-stats"

do_install_append() {
    for f in ${FILES_nfs-utils-stats};do
	rm -vf ${D}${f}
    done
}
