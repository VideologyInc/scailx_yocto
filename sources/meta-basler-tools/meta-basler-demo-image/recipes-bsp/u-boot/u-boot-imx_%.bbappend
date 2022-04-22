# U-boot Makefile seems to have a race when building in parallel (make -j):
# Creating symlink to arch include directory and building target u-boot.lds (which requires this symlink) seems to overlap sometimes, resulting in:
#  /some/path/imx8mm_evk.h:11:10: fatal error: asm/arch/imx-regs.h: No such file or directory
PARALLEL_MAKE = ""
PARALLEL_MAKEINST = ""

FILESEXTRAPATHS_prepend := "${THISDIR}/u-boot-imx:"
SRC_URI_append_imx8mqevk = " file://0001-add-CONFIG_DEFAULT_FDT_FILE-to-imx8mq_evk-in-u-boot.patch"

patch_uboot_defconfig () {
    sed -i -e '/^[	 ]*CONFIG_DEFAULT_FDT_FILE[	 ]*=/d' ${S}/configs/$1
    echo 'CONFIG_DEFAULT_FDT_FILE="'$2'"' >>${S}/configs/$1
}

do_configure_prepend_imx8mqevk () {
    patch_uboot_defconfig imx8mq_evk_defconfig imx8mq-evk-basler-camera.dtb
}

do_configure_prepend_imx8mmevk () {
    patch_uboot_defconfig imx8mm_evk_defconfig imx8mm-evk-basler-camera.dtb
}

do_configure_prepend_imx8mmddr4evk () {
    patch_uboot_defconfig imx8mm_ddr4_evk_defconfig imx8mm-ddr4-evk-basler-camera.dtb
}

