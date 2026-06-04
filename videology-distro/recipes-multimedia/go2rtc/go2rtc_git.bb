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

# New service to detect usb camera live.
SRC_URI += "file://check_usb_live.service"

# New Boson AI model files
SRC_URI += "file://boson/yolov8n_float16.tflite"
SRC_URI += "file://boson/coco.txt"
SRC_URI += "file://boson/thermal_yolov8n_320.tflite"
SRC_URI += "file://boson/thermal.txt"
	
# 2 New json files.
SRC_URI += "file://camera_dict.json"
SRC_URI += "file://camera_gst_dict.json"

# New imx files
SRC_URI += "file://imx/get_imx_features.sh"

SRC_URI += "file://imx/imx678_gray.json"
SRC_URI += "file://imx/imx678_hue.json"
SRC_URI += "file://imx/imx678.json"
SRC_URI += "file://imx/imx900_gray.json"
SRC_URI += "file://imx/imx900.json"
SRC_URI += "file://imx/imx_para_tmp.txt"
SRC_URI += "file://imx/imx_para_values.txt"

SRC_URI += "file://imx/libjsoncpp.so.21"
SRC_URI += "file://imx/vvget"

inherit systemd

SYSTEMD_SERVICE:${PN} = "go2rtc.service"
SYSTEMD_SERVICE:${PN} += "check_usb_live.service"

do_install:append(){
    install -d  ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/go2rtc.service ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/check_usb_live.service ${D}${systemd_system_unitdir}

    install -d ${D}${sysconfdir}/default
    install -m 0644 ${WORKDIR}/go2rtc.yaml ${D}${sysconfdir}/default/

    # Files to default target folder do NOT need to add to {PN}.
    install -m 0755 ${WORKDIR}/create_cams_config.py ${D}${bindir}/
    install -m 0755 ${WORKDIR}/go2rtc-create-cams-config.py ${D}${bindir}/
    install -m 0644 ${WORKDIR}/camera_dict.json ${D}${sysconfdir}/default/
    install -m 0644 ${WORKDIR}/camera_gst_dict.json ${D}${sysconfdir}/default/
    rm -rf ${D}${bindir}/go2rtc_*

    # Boson files will be installed on target /opt/imx8-isp/boson/.
    # Create new folder on Scailx device.
    install -d ${D}/opt/imx8-isp/boson

    # single file only
    # install -m 0755 ${WORKDIR}/boson/coco.txt ${D}/opt/imx8-isp/boson/			

    # Install multiple files in subfolder ~/go2rtc/boson to Scailx device.
    for f in ${WORKDIR}/boson/*; do install -m 0755 $f ${D}/opt/imx8-isp/boson/ ; done
    
    # imx files will be installed to target /opt/imx8-isp/imx/.
    # Create new folder on Scailx device.
    install -d ${D}/opt/imx8-isp/imx

    # Two binary files to standard target folder do not need to add to {PN}.
    install -m 0755 ${WORKDIR}/imx/libjsoncpp.so.21 ${D}${libdir}/
    install -m 0755 ${WORKDIR}/imx/vvget ${D}${bindir}/
    
    # Multiple json and txt + sh files to target /opt/imx8-isp/imx/.
    for f in ${WORKDIR}/imx/*.json; do install -m 0755 $f ${D}/opt/imx8-isp/imx/ ; done
    for f in ${WORKDIR}/imx/*.txt; do install -m 0755 $f ${D}/opt/imx8-isp/imx/ ; done
    for f in ${WORKDIR}/imx/*.sh; do install -m 0755 $f ${D}/opt/imx8-isp/imx/ ; done
}

RDEPENDS:${PN} += "python3-core python3-pyyaml python3-linuxpy python3-v4l2py"
FILES:${PN} += "${bindir}/go2rtc ${systemd_system_unitdir}/system ${confdir}"

# Boson AI model files to non-standard target path need to be added here.
FILES:${PN} += "/opt/imx8-isp/boson/"
FILES:${PN} += "/opt/imx8-isp/boson/yolov8n_float16.tflite"
FILES:${PN} += "/opt/imx8-isp/boson/coco.txt"
FILES:${PN} += "/opt/imx8-isp/boson/thermal_yolov8n_320.tflite"
FILES:${PN} += "/opt/imx8-isp/boson/thermal.txt"

RDEPENDS:${PN} += "bash"
# Also 8 imx files to non-standard target path need to be added here.
FILES:${PN} += "/opt/imx8-isp/imx/"
FILES:${PN} += "/opt/imx8-isp/imx/get_imx_features.sh"
FILES:${PN} += "/opt/imx8-isp/imx/imx678_gray.json"
FILES:${PN} += "/opt/imx8-isp/imx/imx678_hue.json"
FILES:${PN} += "/opt/imx8-isp/imx/imx678.json"
FILES:${PN} += "/opt/imx8-isp/imx/imx900_gray.json"
FILES:${PN} += "/opt/imx8-isp/imx/imx900.json"
FILES:${PN} += "/opt/imx8-isp/imx/imx_para_tmp.txt"
FILES:${PN} += "/opt/imx8-isp/imx/imx_para_values.txt"


INSANE_SKIP:${PN} += "already-stripped"
INHIBIT_PACKAGE_STRIP = "1"


