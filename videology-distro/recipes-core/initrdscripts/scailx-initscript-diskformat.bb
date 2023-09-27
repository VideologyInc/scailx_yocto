DESCRIPTION = "device-specific recovery-script to format emmc or other storage. \
			At minimim needs to have one 'boot' labeled patition large enough to hold 2 kernel copies, \
			and a 'storage' labeled partition as big as possible. "
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " file://diskformat.sh"

S = "${WORKDIR}"

do_install () {
	install -d ${D}${sbindir}
	install -m 0755 ${WORKDIR}/diskformat.sh ${D}${sbindir}/diskformat.sh
}
