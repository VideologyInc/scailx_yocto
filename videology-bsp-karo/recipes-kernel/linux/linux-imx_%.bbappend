FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# remove the NXP preocesses that clobber the defconfig.
deltask copy_defconfig
deltask merge_delta_config

SRC_URI:append = " file://scailx-kmeta;type=kmeta;destsuffix=scailx-kmeta "

SRC_URI:append = " file://of-configfs.patch "

PV:scailx = "${LINUX_VERSION}"