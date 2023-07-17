KBUILD_BUILD_USER := "support"
KBUILD_BUILD_HOST := "karo-electronics.de"

MACHINEOVERRIDES =. "mx8:mx8m:"

BUILD_REPRODUCIBLE_BINARIES ?= "0"
INHERIT:remove = "${@'reproducible_build' if d.getVar('BUILD_REPRODUCIBLE_BINARIES') == '0' else ''}"

require conf/machine/include/imx-base.inc
require conf/machine/include/arm/armv8a/tune-cortexa53.inc

SOC_PREFIX = "imx8m"

SOC_FAMILY ??= ""
SOC_FAMILY:mx8mm-nxp-bsp = "imx8mm"
SOC_FAMILY:mx8mn-nxp-bsp = "imx8mn"
SOC_FAMILY:mx8mp-nxp-bsp = "imx8mp"
MACHINEOVERRIDES =. "${@ "${SOC_FAMILY}:" if "${SOC_FAMILY}" != "" else ""}"

MACHINE_FEATURES += "emmc"
MACHINE_FEATURES:remove = "nxp8987"

MACHINE_USES_VIVANTE_KERNEL_DRIVER_MODULE = "0"
MACHINE_HAS_VIVANTE_KERNEL_DRIVER_SUPPORT = "1"

KARO_BOARD_PMIC ?= "bd71847"

UBOOT_CONFIG ??= "noenv mfg default"
UBOOT_FEATURES ??= ""

# Set U-Boot DTB
KARO_BASEBOARD ??= ""
UBOOT_DTB_NAME = "${@ "${DTB_BASENAME}%s.dtb" % ("-${KARO_BASEBOARD}" if "${KARO_BASEBOARD}" != "" else "")}"

BOOT_TOOLS = "imx-boot-tools"

SPL_BINARY = "spl/u-boot-spl.bin"

# Set imx-mkimage boot target
IMXBOOT_TARGETS ?= "flash_tx8m"
IMX_BOOT_SOC_TARGET ?= "iMX8MM"
ATF_PLATFORM ?= "imx8mm"
ATF_LOAD_ADDR ?= "0x920000"

# Set Serial console
SERIAL_CONSOLES ?= "115200;ttymxc0 115200;ttymxc1 115200;ttymxc2"
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
PREFERRED_PROVIDER_virtual/bootloader = "u-boot-karo"
# PREFERRED_PROVIDER_virtual/kernel = "linux-karo"
PREFERRED_PROVIDER_imx-boot = "imx-boot-karo"

PREFERRED_PROVIDER_ptpd = "ptpd"

# Blacklist evbug kernel module
KERNEL_MODULE_PROBECONF += "evbug"
module_conf_evbug = "blacklist evbug"

