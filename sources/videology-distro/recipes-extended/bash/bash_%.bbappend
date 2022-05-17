FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
SRC_URI_append = " \
	       	  file://bash_aliases\
	       	  file://bash_login\
"
FILES_${PN} += " \
	       ${ROOT_HOME}/.bash_aliases \
	       ${ROOT_HOME}/.bash_login \
"

do_install_append() {
    install -d -m 700 ${D}${ROOT_HOME}
    install -m 644 ${WORKDIR}/bash_aliases ${D}${ROOT_HOME}/.bash_aliases
    install -m 644 ${WORKDIR}/bash_login ${D}${ROOT_HOME}/.bash_login
}
