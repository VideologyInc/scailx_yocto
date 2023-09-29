SUMMARY = "U-boot distro-boot script"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
COMPATIBLE_MACHINE = "(mx8)"

DEPENDS = "u-boot-mkimage-native"

FILESEXTRAPATHS:prepend := "${SCAILX_SCRIPTS_DIRS}:"

SRC_URI = "file://boot.cmd"

do_compile() {
    mkimage -C none -A arm -T script -d "${WORKDIR}/boot.cmd" boot.scr
}

inherit deploy

do_deploy() {
    install -d ${DEPLOYDIR}
    install -m 0644 boot.scr ${DEPLOYDIR}
}

addtask do_deploy after do_compile before do_build