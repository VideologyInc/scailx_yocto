SUMMARY = "A qt5 image with Weston desktop for use with qt creator for app development"

require karo-image-weston.bb

inherit populate_sdk_qt5

IMAGE_INSTALL_append = "${@ "packagegroup-qt5-imx" if "use-nxp-bsp" in d.getVar('MACHINEOVERRIDES').split(':') else "packagegroup-qt5"}"

IMAGE_FEATURES += "qtcreator-debug"

ROOTFS_PARTITION_SIZE = "2097152"
