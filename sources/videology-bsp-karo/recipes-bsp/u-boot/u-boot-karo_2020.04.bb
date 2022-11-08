# Copyright (C) 2013-2020 Lothar Waßmann <LW@KARO-electronics.de>
# based on: meta-imx/meta-bsp/recipes-bsp/u-boot/u-boot-imx_2020.04.bb
#   Copyright (C) 2013-2016 Freescale Semiconductor
#   Copyright 2017-2020 NXP

DESCRIPTION = "i.MX U-Boot suppporting Ka-Ro electronics boards."
HOMEPAGE = "http://www.denx.de/wiki/U-Boot/WebHome"
SECTION = "bootloaders"
DEPENDS += "flex-native bison-native dtc-native"

require recipes-bsp/u-boot/u-boot.inc
inherit fsl-u-boot-localversion

FILESEXTRAPATHS:prepend := "${THISDIR}/${BP}/defconfigs:${THISDIR}/${BP}/cfg:${THISDIR}/${BP}/env:"

PROVIDES += "u-boot"

LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://Licenses/gpl-2.0.txt;md5=b234ee4d69f5fce4486a80fdaf4a4263"
PE = "1"

UBOOT_SRC ?= "git://github.com/karo-electronics/karo-tx-uboot.git;protocol=https"
SRC_URI = "${UBOOT_SRC};branch=${SRCBRANCH}"
SRCBRANCH = "lf_v2020.04-karo"
SRCREV = "d3987c4f40889be73aed0ecde9188bc1d2fdc0c8"

S = "${WORKDIR}/git"
B = "${WORKDIR}/build"

LOCALVERSION ?= "-5.10.9-1.0.0"

SRC_URI:append = "${@ "".join(map(lambda f: " file://%s.cfg" % f, d.getVar('UBOOT_FEATURES').split()))}"

SRC_URI:append:mx8m-nxp-bsp = " file://${MACHINE}_env.txt;subdir=git/board/karo/tx8m"

SRC_URI:append = "${@ " file://dts/${UBOOT_DTB_NAME}".replace(".dtb", ".dts") + ";subdir=git/arch/arm"}"
SRC_URI:append = "${@ " file://dts/${UBOOT_DTB_NAME}".replace(".dtb", "-u-boot.dtsi") + ";subdir=git/arch/arm"}"

SRC_URI:append = " file://${MACHINE}_defconfig.template"
SRC_URI:append = "${@ "".join(map(lambda f: " file://u-boot-cfg.%s" % f, d.getVar('UBOOT_CONFIG').split()))}"

EXTRA_OEMAKE:append = " V=0"

do_patch:append() {
    import shutil

    def concat_file(src, dst):
        bb.note("Appending '%s' to '%s'" % (src, dst))
        r = open(src, 'r')
        f = open(dst, 'a')
        f.write(r.read())

    if d.getVar('UBOOT_CONFIG') != "":
        i = 0
        for config in d.getVar('UBOOT_MACHINE').split():
            j = 0
            for type in d.getVar('UBOOT_CONFIG').split():
                if i == j:
                    c = config.replace("_config", "_defconfig")
                    bb.note("Installing '%s' to '%s'" % (d.getVar('MACHINE') + "_defconfig.template", os.path.join(d.getVar('S'), "configs", c)))
                    shutil.copyfile(d.getVar('MACHINE') + "_defconfig.template", os.path.join(d.getVar('S'), "configs", c))
                    if os.path.exists("u-boot-cfg.%s" % type):
                        bb.note("Appending '%s' specific config to '%s/configs/%s'" % (type, d.getVar('S'), c))
                        concat_file("u-boot-cfg.%s" % type, os.path.join(d.getVar('S'), "configs", c))
                j += 1
            i += 1
    else:
        shutil.copyfile(d.getVar('MACHINE') + "_defconfig.template", os.path.join(d.getVar('S'), "configs", d.getVar('MACHINE') + "_defconfig"))
}

do_configure:prepend() {
    for f in ${UBOOT_FEATURES};do
        if ! [ -f "${WORKDIR}/${f}.cfg" ];then
            bbfatal "UBOOT_FEATURE: '${WORKDIR}/${f}.cfg' not found"
        fi
    done
}

do_configure:append() {
    if [ "${KARO_BASEBOARD}" != "" ];then
        tmpfile="`mktemp cfg-XXXXXX.tmp`"
        if [ -z "$tmpfile" ];then
            bbfatal "Failed to create tmpfile"
        fi
        cat <<EOF >> "$tmpfile"
CONFIG_DEFAULT_DEVICE_TREE="${DTB_BASENAME}-${KARO_BASEBOARD}"
CONFIG_DEFAULT_ENV_FILE="board/\$(VENDOR)/\$(BOARD)/${UBOOT_ENV_FILE}"
EOF
        if [ -n "${UBOOT_CONFIG}" ];then
            for config in ${UBOOT_MACHINE};do
                c="${B}/${config}"
                merge_config.sh -m -r -O "${c}" "${c}/.config" "$tmpfile"
                oe_runmake -C ${c} oldconfig
            done
        else
            merge_config.sh -m -r -O "${B}" "${B}/.config" "$tmpfile"
            oe_runmake -C ${c}/${config} oldconfig
        fi
        rm -f "$tmpfile"

        grep -q "${DTB_BASENAME}-${KARO_BASEBOARD}\.dtb" ${S}/arch/arm/dts/Makefile || \
                sed -i '/^targets /i\
dtb-y += ${DTB_BASENAME}-${KARO_BASEBOARD}.dtb\
' ${S}/arch/arm/dts/Makefile
    fi
}

do_deploy:append:mx8m-nxp-bsp () {
    install -d "${DEPLOYDIR}/${BOOT_TOOLS}"

    # Deploy u-boot-nodtb.bin and dtb file, to be packaged in boot binary by imx-boot
    if [ -n "${UBOOT_CONFIG}" ];then
        i=0
        for config in ${UBOOT_MACHINE};do
            i=$(expr $i + 1)
            j=0
            for type in ${UBOOT_CONFIG};do
                j=$(expr $j + 1)
                if [ $j -eq $i ];then
                    install -m 0644 "${B}/${config}/arch/arm/dts/${UBOOT_DTB_NAME}" "${DEPLOYDIR}/${BOOT_TOOLS}"
                    install -m 0644 "${B}/${config}/u-boot-nodtb.bin" "${DEPLOYDIR}/${BOOT_TOOLS}/u-boot-nodtb.${UBOOT_SUFFIX}-${MACHINE}-${type}"
                fi
            done
            unset  j
        done
        unset  i
    fi
}

do_savedefconfig() {
    if [ -n "${UBOOT_CONFIG}" ];then
        for config in ${UBOOT_MACHINE};do
            bbplain "Saving defconfig to:\n${B}/${config}/defconfig"
            oe_runmake -C ${B}/${config} savedefconfig
        done
    else
        bbplain "Saving defconfig to:\n${B}/defconfig"
        oe_runmake -C ${B} savedefconfig
    fi
}
do_savedefconfig[nostamp] = "1"
addtask savedefconfig after do_configure

addtask do_configure before do_devshell

PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(mx8)"
