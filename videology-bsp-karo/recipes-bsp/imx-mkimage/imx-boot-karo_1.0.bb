# Copyright (C) 2017-2020 NXP

require imx-mkimage_git.inc

DESCRIPTION = "Generate Boot Loader for i.MX 8 device"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"
SECTION = "BSP"

inherit use-imx-security-controller-firmware

IMX_EXTRA_FIRMWARE      = "firmware-imx-8 imx-sc-firmware imx-seco"
IMX_EXTRA_FIRMWARE:mx8m-nxp-bsp = "firmware-imx-8m"
IMX_EXTRA_FIRMWARE:mx8x = "imx-sc-firmware imx-seco"
DEPENDS += " \
    u-boot \
    ${IMX_EXTRA_FIRMWARE} \
    imx-atf \
    ${@bb.utils.contains('MACHINE_FEATURES', 'optee', 'optee-os', '', d)} \
"
# xxd is a dependency of fspi_packer.sh
DEPENDS += "xxd-native"
DEPENDS:append:mx8m-nxp-bsp = " u-boot-mkimage-native dtc-native"
BOOT_NAME = "imx-boot-karo"
PROVIDES = "${BOOT_NAME}"
PROVIDES += "imx-boot"

inherit deploy

# Add CFLAGS with native INCDIR & LIBDIR for imx-mkimage build
CFLAGS = "-O2 -Wall -std=c99 -I ${STAGING_INCDIR_NATIVE} -L ${STAGING_LIBDIR_NATIVE}"

# This package aggregates output deployed by other packages,
# so set the appropriate dependencies
do_compile[depends] += " \
    virtual/bootloader:do_deploy \
    ${@' '.join('%s:do_deploy' % r for r in '${IMX_EXTRA_FIRMWARE}'.split() )} \
    imx-atf:do_deploy \
    ${@bb.utils.contains('MACHINE_FEATURES', 'optee', 'optee-os:do_deploy', '', d)} \
"

SC_FIRMWARE_NAME ?= "scfw_tcm.bin"

ATF_MACHINE_NAME ?= "bl31-${ATF_PLATFORM}.bin"
ATF_MACHINE_NAME:append = "${@bb.utils.contains('MACHINE_FEATURES', 'optee', '-optee', '', d)}"

TOOLS_NAME ?= "mkimage_imx8"

IMX_BOOT_SOC_TARGET ?= "INVALID"

DEPLOY_OPTEE = "${@bb.utils.contains('MACHINE_FEATURES', 'optee', 'true', 'false', d)}"

IMXBOOT_TARGETS ?= \
    "${@bb.utils.contains('UBOOT_CONFIG', 'fspi', 'flash_flexspi', \
        bb.utils.contains('UBOOT_CONFIG', 'nand', 'flash_nand', \
                                                  'flash_multi_cores flash_dcd', d), d)}"

BOOT_STAGING       = "${S}/${IMX_BOOT_SOC_TARGET}"
BOOT_STAGING:mx8m-nxp-bsp  = "${S}/iMX8M"

SOC_FAMILY      = "INVALID"
SOC_FAMILY:mx8-nxp-bsp  = "mx8"
SOC_FAMILY:mx8m-nxp-bsp = "mx8m"

REV_OPTION ?= ""

compile_mx8m() {
    local t="$1"
    bbnote 8MQ/8MM boot binary build
    for ddr_firmware in ${DDR_FIRMWARE_NAME}; do
        bbnote "Copy ddr_firmware: ${ddr_firmware} from ${DEPLOY_DIR_IMAGE} -> ${BOOT_STAGING}"
        install -v "${DEPLOY_DIR_IMAGE}/${ddr_firmware}"               "${BOOT_STAGING}"
    done

    install -v "${DEPLOY_DIR_IMAGE}/signed_dp_imx8m.bin"               "${BOOT_STAGING}"
    install -v "${DEPLOY_DIR_IMAGE}/signed_hdmi_imx8m.bin"             "${BOOT_STAGING}"

    install -v "${DEPLOY_DIR_IMAGE}/${BOOT_TOOLS}/${UBOOT_DTB_NAME}"   "${BOOT_STAGING}"

    bbnote "\
Using standard mkimage from u-boot-tools for FIT image builds. The standard \
mkimage is compatible for this use, and using it saves us from having to \
maintain a custom recipe."
    ln -svf "${STAGING_DIR_NATIVE}${bindir}/mkimage"            "${BOOT_STAGING}/mkimage_uboot"
    install -v "${DEPLOY_DIR_IMAGE}/${BOOT_TOOLS}/${ATF_MACHINE_NAME}" "${BOOT_STAGING}/bl31.bin"

    UBOOT_NAME="u-boot-${MACHINE}.${UBOOT_SUFFIX}-${t}"
    BOOT_CONFIG_MACHINE="${BOOT_NAME}-${MACHINE}.${UBOOT_SUFFIX}-${t}"

    install -v "${DEPLOY_DIR_IMAGE}/u-boot-spl.${UBOOT_SUFFIX}-${MACHINE}-${t}" \
                                                            "${BOOT_STAGING}/u-boot-spl.${UBOOT_SUFFIX}"
    install -v "${DEPLOY_DIR_IMAGE}/${BOOT_TOOLS}/u-boot-nodtb.${UBOOT_SUFFIX}-${MACHINE}-${t}" \
                                                            "${BOOT_STAGING}/u-boot-nodtb.${UBOOT_SUFFIX}"
    install -v "${DEPLOY_DIR_IMAGE}/${UBOOT_NAME}" "${BOOT_STAGING}/u-boot.${UBOOT_SUFFIX}"
}

compile_mx8() {
    local t="$1"
    bbnote 8QM boot binary build

    UBOOT_NAME="u-boot-${MACHINE}.${UBOOT_SUFFIX}-${t}"
    BOOT_CONFIG_MACHINE="${BOOT_NAME}-${MACHINE}.${UBOOT_SUFFIX}-${t}"
    install -v ${DEPLOY_DIR_IMAGE}/${UBOOT_NAME}                 "${BOOT_STAGING}/u-boot.${UBOOT_SUFFIX}"
    if [ -e "${DEPLOY_DIR_IMAGE}/u-boot-spl.${UBOOT_SUFFIX}-${MACHINE}-${t}" ];then
        install -v "${DEPLOY_DIR_IMAGE}/u-boot-spl.${UBOOT_SUFFIX}-${MACHINE}-${t}" \
                                                            "${BOOT_STAGING}/u-boot-spl.${UBOOT_SUFFIX}"
    fi
}

do_compile() {
    install -v -d -m 0775 "${BOOT_STAGING}"

    # Copy TEE binary to SoC target folder to mkimage
    if ${DEPLOY_OPTEE}; then
        install -v "${DEPLOY_DIR_IMAGE}/tee.bin" "${BOOT_STAGING}"
    fi
    for type in ${UBOOT_CONFIG};do
        compile_${SOC_FAMILY} ${type}
        BOOT_CONFIG_MACHINE="${BOOT_NAME}-${MACHINE}.${UBOOT_SUFFIX}-${type}"

        # mkimage for i.MX8
        for target in ${IMXBOOT_TARGETS}; do
            if [ "$target" = "flash_linux_m4_no_v2x" ]; then
                # Special target build for i.MX 8DXL with V2X off
                bbnote "building ${IMX_BOOT_SOC_TARGET} - ${REV_OPTION} V2X=NO ${target}"
                make SOC=${IMX_BOOT_SOC_TARGET} ${REV_OPTION} V2X=NO dtbs=${UBOOT_DTB_NAME} flash_linux_m4
            else
                bbnote "building ${IMX_BOOT_SOC_TARGET} - ${REV_OPTION} ${target}"
                make SOC=${IMX_BOOT_SOC_TARGET} ${REV_OPTION} dtbs=${UBOOT_DTB_NAME} ${target}
            fi
            if [ -e "${BOOT_STAGING}/flash.bin" ]; then
                install -v "${BOOT_STAGING}/flash.bin" "${S}/${BOOT_CONFIG_MACHINE}-${target}"
            fi
        done
    done
}

do_install () {
    install -v -d ${D}/boot

    for type in ${UBOOT_CONFIG};do
        BOOT_CONFIG_MACHINE="${BOOT_NAME}-${MACHINE}.${UBOOT_SUFFIX}-${type}"
        for target in ${IMXBOOT_TARGETS}; do
            install -v -m 0644 ${S}/${BOOT_CONFIG_MACHINE}-${target} ${D}/boot/
        done
    done
}

deploy_mx8m() {
    for t in ${UBOOT_CONFIG};do
        install -v -m 0644 ${DEPLOY_DIR_IMAGE}/u-boot-spl.${UBOOT_SUFFIX}-${MACHINE}-${t} \
                                                             ${DEPLOYDIR}/${BOOT_TOOLS}
    done
    for ddr_firmware in ${DDR_FIRMWARE_NAME}; do
        install -v -m 0644 ${DEPLOY_DIR_IMAGE}/${ddr_firmware}  ${DEPLOYDIR}/${BOOT_TOOLS}
    done
    install -v -m 0644 ${BOOT_STAGING}/signed_dp_imx8m.bin      ${DEPLOYDIR}/${BOOT_TOOLS}
    install -v -m 0644 ${BOOT_STAGING}/signed_hdmi_imx8m.bin    ${DEPLOYDIR}/${BOOT_TOOLS}
    install -v -m 0755 ${BOOT_STAGING}/${TOOLS_NAME}            ${DEPLOYDIR}/${BOOT_TOOLS}
    install -v -m 0755 ${BOOT_STAGING}/mkimage_fit_atf.sh       ${DEPLOYDIR}/${BOOT_TOOLS}
    install -v -m 0755 ${BOOT_STAGING}/mkimage_uboot            ${DEPLOYDIR}/${BOOT_TOOLS}
}

deploy_mx8() {
    install -v -m 0644 ${BOOT_STAGING}/${SECO_FIRMWARE_NAME}    ${DEPLOYDIR}/${BOOT_TOOLS}
    install -v -m 0644 ${BOOT_STAGING}/m4_image.bin             ${DEPLOYDIR}/${BOOT_TOOLS}
    install -v -m 0644 ${BOOT_STAGING}/m4_1_image.bin           ${DEPLOYDIR}/${BOOT_TOOLS}
    install -v -m 0755 ${S}/${TOOLS_NAME}                       ${DEPLOYDIR}/${BOOT_TOOLS}

    for t in ${UBOOT_CONFIG};do
        if [ -e "${DEPLOY_DIR_IMAGE}/u-boot-spl.${UBOOT_SUFFIX}-${MACHINE}-${t}" ];then
            install -v -m 0644 "${DEPLOY_DIR_IMAGE}/u-boot-spl.${UBOOT_SUFFIX}-${MACHINE}-${t}" \
                                                                 "${DEPLOYDIR}/${BOOT_TOOLS}"
        fi
    done
}

do_deploy() {
    install -v -d ${DEPLOYDIR}/${BOOT_TOOLS}

    # copy tee.bin to deploy path
    if "${DEPLOY_OPTEE}"; then
        install -v -m 0644 ${DEPLOY_DIR_IMAGE}/tee.bin          ${DEPLOYDIR}/${BOOT_TOOLS}
    fi

    # copy the tool mkimage to deploy path and sc fw, dcd and uboot
    # copy makefile (soc.mak) for reference
    install -v -m 0644 ${BOOT_STAGING}/soc.mak                  ${DEPLOYDIR}/${BOOT_TOOLS}

    for type in ${UBOOT_CONFIG};do
        deploy_${SOC_FAMILY}
        UBOOT_NAME="u-boot-${MACHINE}.${UBOOT_SUFFIX}-${type}"
        BOOT_CONFIG_MACHINE="${BOOT_NAME}-${MACHINE}.${UBOOT_SUFFIX}-${type}"

        install -v -m 0644 ${DEPLOY_DIR_IMAGE}/${UBOOT_NAME}        ${DEPLOYDIR}/${BOOT_TOOLS}
        # copy the generated boot image to deploy path
        for target in ${IMXBOOT_TARGETS}; do
            # Use first "target" as IMAGE_IMXBOOT_TARGET
            if [ "$IMAGE_IMXBOOT_TARGET" = "" ]; then
                IMAGE_IMXBOOT_TARGET="$target"
                echo "Set boot target as $IMAGE_IMXBOOT_TARGET"
                ln -svf ${BOOT_CONFIG_MACHINE}-${IMAGE_IMXBOOT_TARGET} ${DEPLOYDIR}/${BOOT_NAME}
            fi
            install -v -m 0644 ${S}/${BOOT_CONFIG_MACHINE}-${target} ${DEPLOYDIR}
        done
        ln -svf ${BOOT_CONFIG_MACHINE}-${IMAGE_IMXBOOT_TARGET} ${DEPLOYDIR}/${BOOT_NAME}-${type}
    done
}
addtask deploy before do_build after do_compile

PACKAGE_ARCH = "${MACHINE_ARCH}"
FILES:${PN} = "/boot"

COMPATIBLE_MACHINE = "(mx8)"
