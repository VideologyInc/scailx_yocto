DESCRIPTION = "Create /etc/apt/source/list.d/scailx.list"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

S = "${WORKDIR}"

inherit allarch

# python() {
#     package_arcs = ${@bb.utils.contains('PACKAGE_ARCHS', 'all', 'all', ' '.join(d.getVar('PACKAGE_ARCHS').split()))}
#     arch_vars = ["ALL_MULTILIB_PACKAGE_ARCHS",
#                  "SDK_PACKAGE_ARCHS",
#                 ]

#     opkg_index_cmd = bb.utils.which(os.getenv('PATH'), "opkg-make-index")

#     if not os.path.exists(os.path.join(self.deploy_dir, "Packages")):
#         open(os.path.join(self.deploy_dir, "Packages"), "w").close()

#     index_cmds = set()
#     index_sign_files = set()
#     for arch_var in arch_vars:
#         bb.warn("arch_var: %s" % arch_var)
# }

do_install() {
    install -d ${D}/etc/apt/source.list.d
    touch ${WORKDIR}/scailx.list
    echo "deb https://scailx-ppa.org.com/${DISTRO}/${DISTRO_VERSION}/${DPKG_ARCH} buster main" > ${WORKDIR}/scailx.list
    echo "deb http://scailx.com/debian/ buster main" > ${WORKDIR}/scailx.list
    echo "deb http://scailx.com/debian/ buster main" > ${WORKDIR}/scailx.list
    echo "deb http://scailx.com/debian/ buster main" > ${WORKDIR}/scailx.list

    install -m 0644 ${WORKDIR}/scailx.list ${D}/etc/apt/source.list.d
}

FILES_${PN} = "/etc/apt/source.list.d/scailx.list"
