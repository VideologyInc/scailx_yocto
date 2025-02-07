LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

SRC_URI = "git://github.com/lwfinger/rtw88.git;protocol=https;branch=master"

# Modify these as desired
SRCREV = "c56536cfa522b42cfa73c2c4332000e6aa5bcf97"

S = "${WORKDIR}/git"

inherit module

EXTRA_OEMAKE:append:task-install = " -C ${STAGING_KERNEL_DIR} M=${S}"
EXTRA_OEMAKE += "KSRC=${STAGING_KERNEL_DIR}"

