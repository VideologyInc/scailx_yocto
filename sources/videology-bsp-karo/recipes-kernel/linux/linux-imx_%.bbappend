FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " file://defconfig"

# remove the NXP preocesses that clobber the defconfig.
deltask copy_defconfig
deltask merge_delta_config
