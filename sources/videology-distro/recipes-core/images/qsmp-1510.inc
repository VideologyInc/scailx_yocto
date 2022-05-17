ROOTFS_PARTITION_SIZE ??= "32*1024"
IMAGE_ROOTFS_MAXSIZE ?= "${ROOTFS_PARTITION_SIZE}"

IMAGE_FSTYPES_remove = "ubi"
IMAGE_FSTYPES_append = " multivol_ubi"

# UBI partition size in KiB
UBI_PARTITION_SIZE = "128512"

MULTIUBI_VOLUMES ?= " \
        rootfs \
        env \
        dtb \
        kernel \
        userfs \
"
export MKUBIFS_ARGS = "-F -x zlib -m 2048 -e ${@d.getVar('MKUBIFS_LEB_SIZE')}"

export UBIVOL_SIZE_rootfs = "32MiB"

export UBIVOL_SIZE_env = "128KiB"

export UBIVOL_TYPE_dtb = "static"
export UBIVOL_SIZE_dtb = "128KiB"
export UBIVOL_IMAGE_dtb = "stm32mp151a-qsmp-1510-qsbase1.dtb"

export UBIVOL_TYPE_kernel = "static"
export UBIVOL_SIZE_kernel = "16MiB"
export UBIVOL_IMAGE_kernel = "uImage"

export UBIVOL_SIZE_userfs = "${@128*1024 - 32*1024 - 2*128 - 16*1024}KiB"
export UBIVOL_FLAGS_userfs = "autoresize"
