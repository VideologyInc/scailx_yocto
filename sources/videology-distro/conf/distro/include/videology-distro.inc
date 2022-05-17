POKY_DISTRO ??= "poky"
require conf/distro/${POKY_DISTRO}.conf

POKY_DEFAULT_DISTRO_FEATURES_remove = "\
        largefile \
        opengl \
        ptest \
        vulkan \
        wayland \
"

DISTRO_FEATURES_DEFAULT_remove = "\
        3g \
        argp \
        bluetooth \
        ipv6 \
        nfc \
        pcmcia \
        wifi \
        x11 \
        zeroconf \
"

DISTRO_FEATURES_append = "\
        dhcpcd \
        ldconfig \
        pam \
        telnet-login \
"

ENABLE_PARTITIONS_IMAGE ?= "${@ bb.utils.contains('MACHINE_FEATURES', 'emmc', "1", "0", d)}"
PARTITIONS_IMAGE ?= "${@ "karo-image-bootfs karo-image-userfs" if ${ENABLE_PARTITIONS_IMAGE} == 1 else ""}"

DISTRO_FEATURES_DEFAULT_append_mx8  = " dsi83"
DISTRO_FEATURES_DEFAULT_append_tx8m = " pcie"
DISTRO_FEATURES_DEFAULT_append_qs8m = " csi-camera imx219 raspi-display tc358867"
DISTRO_FEATURES_DEFAULT_append_qsxm = " basler csi-camera pcie raspi-display tc358867"
DISTRO_FEATURES_DEFAULT_append_qsxp = " basler csi-camera pcie raspi-display tc358867"
DISTRO_FEATURES_DEFAULT_append_tx8p = " lvds"

VIRTUAL-RUNTIME_base-utils-hwclock = "util-linux-hwclock"
VIRTUAL-RUNTIME_dev_manager ?= "udev"
VIRTUAL-RUNTIME_init_manager ?= "${@bb.utils.contains('DISTRO_FEATURES',"systemd","systemd","sysvinit",d)}"

PREFERRED_PROVIDER_opencl-headers = "imx-gpu-viv"

VOLATILE_LOG_DIR = "no"

MACHINE_FIRMWARE_remove = "${@ bb.utils.contains('DISTRO_FEATURES', 'wifi', "", "linux-firmware-ath10k", d)}"

ROOTFS_PARTITION_SIZE ?= "${@bb.utils.contains('MACHINE_FEATURES',"nand", \
                      65536-${IMAGE_ROOTFS_EXTRA_SPACE},128*1024,d)}"

# Remove some image types we don't need
IMAGE_FSTYPES_remove = "sdcard.bz2 wic.bz2 wic.bmap"

# mask out sysvinit recipes from meta-imx layer
BBMASK_use-nxp-bsp += ".*/meta-imx/.*/sysvinit/.*"

# mask out recipe breaking the weston desktop without systemd
BBMASK += ".*/meta-imx/meta-bsp/recipes-graphics/wayland/weston-init.bbappend"
