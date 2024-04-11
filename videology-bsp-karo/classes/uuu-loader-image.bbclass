# Class that creates a zip file runnable with NXP's new UUU utility.
# The zip contains a copy of uuu program for convenience, the
# uuu script, card-image to flash and a fastboot compatible u-boot
# image to run on the device.
# USAGE:
#       uuu <image.zip>
# This will run the utility, which will chech for compatible devices
# (NXP imx devices in SDP mode), and will start flashing to the image.
#       uuu -d <image.zip>
# This will run in daemon mode (Runs continuously). Whenever a new
# compatible device is detected, it will start flashing it.

inherit image_types

CARD_ONDISK = "mmcblk"
ZIP_FOLDER = "${WORKDIR}/uuuimg"

do_image_uuuimg[depends] += "parted-native:do_populate_sysroot zip-native:do_populate_sysroot ${IMAGE_BOOTLOADER}:do_deploy virtual/kernel:do_build"
# u-boot-imx-uuu:do_deploy u-boot-imx:do_deploy "

UUU_VERSION = "1.2.91"

IMAGE_TYPEDEP:uuuimg = "wic"
CARD_IMAGE ?= "${IMAGE_LINK_NAME}.wic"

UBOOT_UUU ?= "${DEPLOY_DIR_IMAGE}/imx-boot-karo-mfg"
UBOOT     ?= "${DEPLOY_DIR_IMAGE}/imx-boot"


IMAGE_CMD:uuuimg () {

    cd ${IMGDEPLOYDIR}

    for f in $UUU ${CARD_IMAGE} ${UBOOT_UUU} ${UBOOT}; do
        install "$f" ${ZIP_FOLDER}
    done

    uboot=`basename ${UBOOT}`
    uboot_uuu=`basename ${UBOOT_UUU}`
    image=`basename ${CARD_IMAGE}`

    echo "uuu_version ${UUU_VERSION}

SDPS: boot -f ${uboot_uuu}
FB: flash -raw2sparse all ${image}
FB: flash bootloader ${uboot}
### Set default u-boot environment
FB: ucmd env default -a
FB: ucmd setenv bootdelay 1
FB: ucmd saveenv
FB: ucmd mmc partconf \${emmc_dev} \${emmc_boot_ack} 1 0
FB: done
" > "${ZIP_FOLDER}/uuu.auto"
    zip -r -1 -j "${IMAGE_NAME}-uuu.zip" "${ZIP_FOLDER}"
}

do_image_uuuimg[cleandirs] += "${ZIP_FOLDER}"
