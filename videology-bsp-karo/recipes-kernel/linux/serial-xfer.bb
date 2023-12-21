DESCRIPTION = "Serial transfer unitility to help crosslink send proper visca commands."

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"


SRC_URI += "file://CMakeLists.txt;subdir=${S}"
SRC_URI += "file://serial-xfer.c;subdir=${S}"

inherit cmake

FILES:${PN} += "${base_bindir}/"
