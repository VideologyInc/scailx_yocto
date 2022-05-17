LICENSE = "MIT"

IMAGE_PREPROCESS_COMMAND_append = " image_rootfs_image_clean_task;"

inherit core-image features_check rootfs-postprocess
inherit ${@ "flashlayout-stm32mp" if 'stm32mp1' in d.getVar('MACHINEOVERRIDES').split(":") else ""}

require ${@ "st-machine-common-stm32mp.inc" if 'stm32mp1' in d.getVar('MACHINEOVERRIDES').split(":") else ""}
require ${@ "videology-bsp.inc" if 'use-nxp-bsp' in d.getVar('MACHINEOVERRIDES').split(':') else ""}

REQUIRED_DISTRO_FEATURES ??= ""

ROOTFS_POSTPROCESS_COMMAND_remove = "\
        rootfs_update_timestamp; \
        ${@bb.utils.contains('DISTRO_FEATURES','read-only-rootfs','empty_var_volatile;','',d)} \
"

IMAGE_FEATURES_append = " tools-debug"

IMAGE_INSTALL_append = " \
        alsa-utils-aplay \
        alsa-utils-amixer \
        alsa-utils-alsamixer \
        bash \
        canutils \
        coreutils \
        ${@bb.utils.contains('DISTRO_FEATURES','dhcpcd','dhcpcd','',d)} \
        dosfstools \
        e2fsprogs \
        e2fsprogs-resize2fs \
        file \
        gdbserver \
        i2c-tools \
        kmod \
        ldd \
        less \
        libstdc++ \
        libgpiod \
        libgpiod-tools \
        ${@bb.utils.contains('DISTRO_FEATURES','wifi','linux-firmware-pcie8997','',d)} \
        lsof \
        mmc-utils \
        ${@bb.utils.contains('MACHINE_FEATURES',"nand"," mtd-utils","",d)} \
        ncurses \
        nfs-utils \
        ntp \
        ntpdate \
        parted \
        ${@bb.utils.contains('DISTRO_FEATURES','pcie','pciutils','',d)} \
        rsync \
        strace \
        tcp-wrappers \
        tzdata \
        util-linux-fsck \
        util-linux-mkfs \
        util-linux-mount \
        util-linux-setterm \
        ${@bb.utils.contains('DISTRO_FEATURES','imx219','v4l-utils','',d)} \
        ${@bb.utils.contains('DISTRO_FEATURES','wifi','wpa-supplicant','',d)} \
"

DEPENDS_append = " cross-localedef-native qemuwrapper-cross"
IMAGE_INSTALL_append = " karo-devicetrees"

IMAGE_INSTALL_append = " kernel-modules"
DEPENDS_append_use-mainline-bsp = " u-boot-karo"
DEPENDS_append_use-nxp-bsp = " imx-boot-karo uuu-script-template"

PREFERRED_PROVIDER_fdisk = "util-linux"

IMAGE_ROOTFS_ALIGNMENT ?= "${@bb.utils.contains('MACHINE_FEATURES',"nand",128*1024,1024,d)}"
IMAGE_ROOTFS_EXTRA_SPACE = "${@bb.utils.contains('MACHINE_FEATURES',"nand",2*128,0,d)}"
IMAGE_OVERHEAD_FACTOR = "1"
IMAGE_ROOTFS_SIZE ?= "${@${ROOTFS_PARTITION_SIZE}-(${IMAGE_ROOTFS_EXTRA_SPACE})}"

IMAGE_FSTYPES_remove = "tar.gz"
IMAGE_FSTYPES_append = " tar.bz2"
IMAGE_FSTYPES_append = "${@bb.utils.contains('MACHINE_FEATURES',"emmc"," ext4","",d)}"
IMAGE_FSTYPES_append = "${@bb.utils.contains('MACHINE_FEATURES',"nand"," ubi","",d)}"

# mkfs.ext4 parameters
# set inode size to 256 to support timestamps beyond 2038-01-19
EXTRA_IMAGECMD_ext4 += "-I 256"

# mkubifs parameters
MKUBIFS_LEB_SIZE = "126976"
MKUBIFS_LEB_CNT = "${@int(((${IMAGE_ROOTFS_SIZE} + ${IMAGE_ROOTFS_EXTRA_SPACE}) * 1024 + ${MKUBIFS_LEB_SIZE} - 1) / ${MKUBIFS_LEB_SIZE})}"
MKUBIFS_ARGS ?= "-F -x zlib -m 2048 -e ${MKUBIFS_LEB_SIZE} -c ${MKUBIFS_LEB_CNT}"

UBINIZE_ARGS ?= "-m 2048 -s 2048 -p 128KiB"
