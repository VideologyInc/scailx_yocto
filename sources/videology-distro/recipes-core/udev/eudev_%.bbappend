inherit relative_symlinks
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI_append = "\
	       file://rtcsymlink.sh \
	       file://rtcsymlink.rule \
"
INITSCRIPT_PARAMS = "start 02 S ."

do_install_append() {
    install -v -m 0755 -d ${D}${nonarch_base_libdir}/udev
    install -v -m 0755 ${WORKDIR}/rtcsymlink.sh ${D}${nonarch_base_libdir}/udev/rtcsymlink.sh
    install -v -m 0755 ${WORKDIR}/rtcsymlink.rule ${D}${nonarch_base_libdir}/udev/rules.d/51-rtcsymlink.rules
}
