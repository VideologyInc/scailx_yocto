# Recipe created by recipetool
# This is the basis of a recipe and may need further editing in order to be fully functional.
# (Feel free to remove these comments when editing.)

# WARNING: the following LICENSE and LIC_FILES_CHKSUM values are best guesses - it is
# your responsibility to verify that the values are complete and correct.
#
# The following license files were not able to be identified and are
# represented as "Unknown" below, you will need to check them yourself:
#   LICENSE
#
# NOTE: multiple licenses have been detected; they have been separated with &
# in the LICENSE value for now since it is a reasonable assumption that all
# of the licenses apply. If instead there is a choice between the multiple
# licenses then you should change the value to separate the licenses with |
# instead of &. If there is any doubt, check the accompanying documentation
# to determine which situation is applicable.
LICENSE = "GPL-3.0-only & MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=1ebbd3e34237af26da5dc08a4e440464 \
                    file://ezxml/license.txt;md5=d4cda95c0365c4d0c092701007508eab"

FILESEXTRAPATHS:prepend := "${THISDIR}/files/:"

SRC_URI = "git://github.com/roleoroleo/onvif_simple_server.git;protocol=https;branch=master \
           file://0001-fix-makefile.patch \
           file://0002-ptz_service-fix-printf.patch \
           file://0003-update-profile-values.patch \
           file://0004-add-imaging-service.patch \
           "

SRC_URI += "file://onvif_simple_server.conf"
SRC_URI += "file://snapshot.sh"
SRC_URI += "file://onvif-simple-server.service"
SRC_URI += "file://lighttpd.conf"
SRC_URI += "file://imaging.sh"

# Modify these as desired
PV = "0.0.4"
SRCREV = "0130fb5f035ec42f77481b6920bcb975086797aa"

S = "${WORKDIR}/git"

export HAVE_MBEDTLS = "1"

TARGET_CC_ARCH += "${LDFLAGS}"

DEPENDS += "libtomcrypt"
DEPENDS += "${@bb.utils.contains('HAVE_MBEDTLS', '1', 'mbedtls', '', d)}"
RDEPENDS:${PN} = "lighttpd lighttpd-module-cgi bash"
# RDEPENDS:${PN} = "busybox-httpd"
onvif_dir = "${prefix}/local"

do_install() {
	INSTALL_DIR=${D}${onvif_dir}
	install -d ${INSTALL_DIR}/www/onvif/scripts

	install -m 0755 ${WORKDIR}/snapshot.sh ${INSTALL_DIR}/www/onvif/scripts/
	install -m 0755 ${WORKDIR}/imaging.sh ${INSTALL_DIR}/www/onvif/scripts/

	install -m 0755 ${S}/onvif_simple_server    $INSTALL_DIR/www/onvif/
	ln -sf ./onvif_simple_server                $INSTALL_DIR/www/onvif/device_service
	ln -sf ./onvif_simple_server                $INSTALL_DIR/www/onvif/events_service
	ln -sf ./onvif_simple_server                $INSTALL_DIR/www/onvif/media_service
	ln -sf ./onvif_simple_server                $INSTALL_DIR/www/onvif/ptz_service

	cp -R ${S}/device_service_files             $INSTALL_DIR/www/onvif
	cp -R ${S}/events_service_files             $INSTALL_DIR/www/onvif
	cp -R ${S}/generic_files                    $INSTALL_DIR/www/onvif
	cp -R ${S}/media_service_files              $INSTALL_DIR/www/onvif
	cp -R ${S}/ptz_service_files                $INSTALL_DIR/www/onvif

	install -d ${D}${bindir}
	install -m 0755 ${S}/wsd_simple_server ${D}${bindir}/
	install -d ${D}${sysconfdir}/wsd_simple_server
	cp -R ${S}/wsd_files/* ${D}${sysconfdir}/wsd_simple_server

	install -m 0755 ${S}/onvif_notify_server ${D}${bindir}/
	install -d ${D}${sysconfdir}/onvif_notify_server
	cp -R ${S}/notify_files/* ${D}${sysconfdir}/onvif_notify_server

	install -T -m 0644 ${WORKDIR}/onvif_simple_server.conf ${D}${sysconfdir}/onvif_simple_server.conf
	install -T -m 0644 ${WORKDIR}/lighttpd.conf ${D}${sysconfdir}/lighttpd_onvif_ss.conf

    install -d ${D}${systemd_system_unitdir}
	install -m 0755 ${WORKDIR}/onvif-simple-server.service ${D}${systemd_system_unitdir}/
}

inherit systemd
SYSTEMD_SERVICE:${PN} = "onvif-simple-server.service"
CONFFILES_${PN} = "${sysconfdir}/onvif_simple_server.conf ${D}${sysconfdir}/lighttpd_onvif_ss.conf"

FILES:${PN} += "${onvif_dir}/www/onvif ${sysconfdir} ${bindir} ${systemd_system_unitdir}"
INSANE_SKIP:${PN} += "already-stripped"
