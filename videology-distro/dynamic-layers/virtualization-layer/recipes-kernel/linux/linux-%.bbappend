FILESEXTRAPATHS:prepend := "${THISDIR}/linux-yocto:"

SRC_URI += "${@bb.utils.contains('DISTRO_FEATURES', 'virtualization', ' file://podman.scc', '', d)}"

