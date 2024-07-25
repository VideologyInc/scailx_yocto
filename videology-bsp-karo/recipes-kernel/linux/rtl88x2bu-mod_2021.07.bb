LICENSE = "Proprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=ab842b299d0a92fb908d6eb122cd6de9"

SRC_URI = "git://github.com/morrownr/88x2bu-20210702.git;protocol=https;branch=main"

# Modify these as desired
PV = "1.0+git${SRCPV}"
SRCREV = "62f3a86a2687fe98bd441e0aff5adf87d95c238a"

S = "${WORKDIR}/git"

inherit module

EXTRA_OEMAKE:append:task-install = " -C ${STAGING_KERNEL_DIR} M=${S}"
EXTRA_OEMAKE += "KSRC=${STAGING_KERNEL_DIR}"

