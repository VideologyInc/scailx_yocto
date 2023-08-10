FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " file://btrfs.cfg"

KERNEL_CONFIG_FRAGMENTS += " ${WORKDIR}/btrfs.cfg "