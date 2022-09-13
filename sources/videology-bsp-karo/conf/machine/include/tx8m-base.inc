KBUILD_BUILD_USER := "support"
KBUILD_BUILD_HOST := "karo-electronics.de"

MACHINEOVERRIDES =. "mx8:mx8m:tx8m:"

BUILD_REPRODUCIBLE_BINARIES ?= "0"
INHERIT_remove = "${@'reproducible_build' if d.getVar('BUILD_REPRODUCIBLE_BINARIES') == '0' else ''}"

# IMAGE_CLASSES += "st-partitions-image"

require conf/machine/include/imx-base.inc
require conf/machine/include/tune-cortexa53.inc

MACHINE_FEATURES += "emmc"
MACHINE_FEATURES_remove = "nxp8987"

MACHINE_USES_VIVANTE_KERNEL_DRIVER_MODULE = "0"
MACHINE_HAS_VIVANTE_KERNEL_DRIVER_SUPPORT = "1"

KARO_BOARD_PMIC ?= "bd71847"

UBOOT_CONFIG ??= "noenv mfg default"
BOOT_TOOLS = "imx-boot-tools"

SPL_BINARY = "spl/u-boot-spl.bin"

# Set imx-mkimage boot target
IMXBOOT_TARGETS ?= "flash_tx8m"
IMX_BOOT_SOC_TARGET ?= "iMX8MM"
ATF_PLATFORM ?= "imx8mm"
ATF_LOAD_ADDR ?= "0x920000"

# Set Serial console
SERIAL_CONSOLES = "115200;ttymxc0 115200;ttymxc1 115200;ttymxc2"
# for imx-atf
IMX_BOOT_UART_BASE ?= "0x30860000"

# Set DDR FIRMWARE
DDR_FIRMWARE_NAME ?= " \
        ddr3_imem_1d.bin \
        ddr3_dmem_1d.bin \
"

IMAGE_BOOTLOADER = "imx-boot-karo"

LOADADDR = ""
UBOOT_SUFFIX = "bin"
UBOOT_MAKE_TARGET = ""

IMX_DEFAULT_BSP = "nxp"
PREFERRED_PROVIDER_virtual/bootloader_imx = "u-boot-imx"
PREFERRED_PROVIDER_virtual/kernel_imx = "linux-imx"

PREFERRED_PROVIDER_ptpd = "ptpd"

# Blacklist evbug kernel module
KERNEL_MODULE_PROBECONF += "evbug"
module_conf_evbug = "blacklist evbug"
