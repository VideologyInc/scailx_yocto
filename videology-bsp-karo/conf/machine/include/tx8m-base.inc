
MACHINEOVERRIDES =. "mx8:mx8m:"

# IMAGE_CLASSES += "st-partitions-image"

require conf/machine/include/imx-base.inc
require conf/machine/include/arm/armv8a/tune-cortexa53.inc

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
PREFERRED_PROVIDER_virtual/bootloader = "u-boot-karo"
# PREFERRED_PROVIDER_virtual/kernel = "linux-karo"
PREFERRED_PROVIDER_imx-boot = "imx-boot-karo"

PREFERRED_PROVIDER_ptpd = "ptpd"

# Blacklist evbug kernel module
KERNEL_MODULE_PROBECONF += "evbug"
module_conf_evbug = "blacklist evbug"

# PREFERRED_VERSION_isp-imx                 = "4.2.2.14+basler"
# PREFERRED_VERSION_basler-camera           = "4.2.2.14+basler"
# PREFERRED_VERSION_kernel-module-isp-vvcam = "4.2.2.14+basler"

# PREFERRED_PROVIDER_opencl-headers:imxgpu = "imx-gpu-viv"
# PREFERRED_PROVIDER_opencl-icd-loader:imxgpu = "imx-gpu-viv"
# PREFERRED_PROVIDER_virtual/libgles3:imxgpu3d = "imx-gpu-viv"

# PREFERRED_VERSION_gstreamer1.0-plugins-base = "1.20.0.imx"
# PREFERRED_VERSION_gstreamer1.0-plugins-bad  = "1.20.0.imx"
# PREFERRED_VERSION_gstreamer1.0-plugins-good = "1.20.0.imx"
# PREFERRED_VERSION_gstreamer1.0-plugins-good-qt = "1.20.0.imx"
# PREFERRED_VERSION_gstreamer1.0-plugins-ugly = "1.20.1"
# PREFERRED_VERSION_gstreamer1.0              = "1.20.0.imx"
# PREFERRED_VERSION_gstreamer1.0-libav        = "1.20.1"
# PREFERRED_VERSION_gstreamer1.0-rtsp-server  = "1.20.1"

# PREFERRED_VERSION_gstreamer1.0-plugins-base:mx8-nxp-bsp = "1.20.0.imx"
# PREFERRED_VERSION_gstreamer1.0-plugins-bad:mx8-nxp-bsp  = "1.20.0.imx"
# PREFERRED_VERSION_gstreamer1.0-plugins-good:mx8-nxp-bsp = "1.20.0.imx"
# PREFERRED_VERSION_gstreamer1.0-plugins-good-qt:mx8-nxp-bsp = "1.20.0.imx"
# PREFERRED_VERSION_gstreamer1.0-plugins-ugly:mx8-nxp-bsp = "1.20.1"
# PREFERRED_VERSION_gstreamer1.0:mx8-nxp-bsp              = "1.20.0.imx"
# PREFERRED_VERSION_gstreamer1.0-libav:mx8-nxp-bsp        = "1.20.1"
# PREFERRED_VERSION_gstreamer1.0-rtsp-server:mx8-nxp-bsp  = "1.20.1"

# PREFERRED_VERSION_opengl-es-cts:aarch64 = "3.2.7.0"

# Set a more generic tuning for code reuse across parts
# taken from fsl-imx-base.inc
DEFAULTTUNE:mx8-nxp-bsp    ?= "armv8a-crc-crypto"
DEFAULTTUNE:mx8m-nxp-bsp   ?= "armv8a-crc-crypto"
