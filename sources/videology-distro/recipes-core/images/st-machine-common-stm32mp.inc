#@DESCRIPTION: Common Machine configuration for STM32 systems

require st-machine-flashlayout-stm32mp.inc
require ${@ "qsmp-1510.inc" if 'qsmp-1510' in d.getVar('MACHINEOVERRIDES').split(":") else ""}

# Don't include kernels in standard images
#RDEPENDS_${KERNEL_PACKAGE_NAME}-base = ""

#IMAGE_FSTYPES ?= "tar.bz2"
IMAGE_FSTYPES_append = "${@bb.utils.contains('MACHINE_FEATURES',"emmc"," ext4","",d)}"

# Define image to use for extra partitions
STM32MP_BOOTFS_IMAGE = "karo-image-bootfs"
STM32MP_BOOTFS_LABEL ?= "bootfs"
STM32MP_BOOTFS_MOUNTPOINT_IMAGE ?= "/boot"
STM32MP_USERFS_IMAGE = "karo-image-userfs"
STM32MP_USERFS_LABEL ?= "userfs"
STM32MP_USERFS_MOUNTPOINT_IMAGE ?= "/usr/local"

# Define extra partition to build
PARTITIONS_MOUNTPOINT_IMAGE  = "${STM32MP_BOOTFS_MOUNTPOINT_IMAGE}"
PARTITIONS_MOUNTPOINT_IMAGE += "${STM32MP_USERFS_MOUNTPOINT_IMAGE}"

# Provide list of partition to mount
MOUNT_PARTITIONS_LIST  = "${STM32MP_BOOTFS_LABEL},${STM32MP_BOOTFS_MOUNTPOINT_IMAGE}"
MOUNT_PARTITIONS_LIST += "${STM32MP_USERFS_LABEL},${STM32MP_USERFS_MOUNTPOINT_IMAGE}"

# Define image partition size (supposed to be set as max size in image recipe)
# Proposed value for bootfs is 64MB
BOOTFS_PARTITION_SIZE = "65536"
# Proposed value for rootfs should fit our highest constraint: NAND size (128MiB)
# For optee bootscheme we have the maximum partitions:
#   FSBL1 + SSBL + SSBL2 + TEEH + TEED + TEEX + Multivolume UBI = NAND size
#   Multivolume UBI = 128MiB - (2MiB + 2MiB + 2MiB + 512KiB + 512KiB + 512KiB) = 1016.5MiB
# With multivolume UBI split:
#   Multivolume UBI > uboot_config + uboot_config_r + bootfs + rootfs + userfs + UBI Overhead
# From http://www.linux-mtd.infradead.org/doc/ubi.html#L_overhead, we compute
# the UBI overhead for our NAND:
#   (20*4096/1024 + 4) * 256KiB + (256KiB - 248KiB) * (1016.5MiB/256KiB - 20*4096/1024 - 4) = 53360KiB
# In addition, for each UBIFS, our NAND consummed 9 extra eraseblocks
# So:
#   rootfs < Multivolume UBI - (rootfs + env + dtb + kernel + userfs + UBI Overhead + 4 * 9*eraseblocks)
#   rootfs < 120.5MiB - (256KiB + 64MiB + 16MiB + 128MiB + 53360KiB + 4 * 9 * 256KiB)
# Proposed value for rootfs is 2GiB
ROOTFS_PARTITION_SIZE ??= "2097152"
# Proposed value for userfs is 128MB (4*32MB)
USERFS_PARTITION_SIZE ??= "131072"

# Define the max size for ROOTFS image being built
# On other image partition such settings is directly done in image recipe
IMAGE_ROOTFS_MAXSIZE ?= ""
