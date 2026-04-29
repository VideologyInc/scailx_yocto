DESCRIPTION = "Ultimate camera streaming application with support RTSP, RTMP, HTTP-FLV, WebRTC, MSE, HLS, MP4, MJPEG, HomeKit, FFmpeg, etc."
HOMEPAGE = "https://github.com/AlexxIT/go2rtc"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRCNAME = "go2rtc"

SRCREV = "fa580c516efa95770be2137855c3c3cf19b8092b"
GO_IMPORT = "github.com/AlexxIT/go2rtc"
SRC_URI = "git://${GO_IMPORT};branch=master;protocol=https"
SRC_URI += "file://0001-remove-non-scailx-inputs.patch;patchdir=src/${GO_IMPORT}"

PV = "1.9.9"
GO_EXTRA_LDFLAGS = "-s -w"
export CGO_ENABLED = "0"
GO_INSTALL = "${GO_IMPORT}"
do_compile[network] = "1"

# build executable instead of shared object
GO_LINKSHARED = ""
# GOBUILDFLAGS:remove = "-buildmode=pie"

inherit go-mod

FILES:${PN} += "${bindir}/go2rtc"

# RM_WORK_EXCLUDE += "${PN}"

SRC_URI += "file://go2rtc.yaml"
SRC_URI += "file://go2rtc.service"
SRC_URI += "file://go2rtc-create-cams-config.py"
SRC_URI += "file://create_cams_config.py"

# New Boson AI model files
SRC_URI += "file://boson/yolov8n_float16.tflite"
SRC_URI += "file://boson/coco.txt"
SRC_URI += "file://boson/thermal_yolov8n_320.tflite"
SRC_URI += "file://boson/thermal.txt"
	
# 2 New json files.
SRC_URI += "file://camera_dict.json"
SRC_URI += "file://camera_gst_dict.json"

inherit systemd

SYSTEMD_SERVICE:${PN} = "go2rtc.service"

do_install:append(){
    install -d  ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/go2rtc.service ${D}${systemd_system_unitdir}

    install -d ${D}${sysconfdir}/default
    install -m 0644 ${WORKDIR}/go2rtc.yaml ${D}${sysconfdir}/default/

    install -m 0755 ${WORKDIR}/create_cams_config.py ${D}${bindir}/
    install -m 0755 ${WORKDIR}/go2rtc-create-cams-config.py ${D}${bindir}/
    install -m 0644 ${WORKDIR}/camera_dict.json ${D}${sysconfdir}/default/
    install -m 0644 ${WORKDIR}/camera_gst_dict.json ${D}${sysconfdir}/default/
    rm -rf ${D}${bindir}/go2rtc_*

    # Create new folder on Scailx device.
    install -d ${D}/opt/imx8-isp/boson

    # single file only
    # install -m 0755 ${WORKDIR}/boson/coco.txt ${D}/opt/imx8-isp/boson/			

    # Install multiple files in subfolder ~/go2rtc/boson to Scailx device.
    for f in ${WORKDIR}/boson/*; do install -m 0755 $f ${D}/opt/imx8-isp/boson/ ; done
}

RDEPENDS:${PN} += "python3-core python3-pyyaml"
FILES:${PN} += "${bindir}/go2rtc ${systemd_system_unitdir}/system ${confdir}"

# Ensure the newly installed AI model files are included in the package (${PN})
FILES:${PN} += "/opt/imx8-isp/boson/"
FILES:${PN} += "/opt/imx8-isp/boson/yolov8n_float16.tflite"
FILES:${PN} += "/opt/imx8-isp/boson/coco.txt"
FILES:${PN} += "/opt/imx8-isp/boson/thermal_yolov8n_320.tflite"
FILES:${PN} += "/opt/imx8-isp/boson/thermal.txt"

INSANE_SKIP:${PN} += "already-stripped"
INHIBIT_PACKAGE_STRIP = "1"


