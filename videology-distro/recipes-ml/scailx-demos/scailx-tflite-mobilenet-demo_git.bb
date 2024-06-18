DESCRIPTION="tflite mobilenet calssification demo"
LICENSE="MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "git://git@github.com/VideologyInc/python-tflite-mobilenet-demo.git;branch=main;protocol=ssh;lfs=1"
SRCREV = "693ce22b9e5dda141da4f8098b55c1cccc480dd9"

S = "${WORKDIR}/git"

inherit python3native

RDEPENDS:${PN} += "python3-opencv tensorflow-lite gstreamer1.0-python gstreamer1.0-rtsp-server imx-gst1.0-plugin libgstrtspserver-1.0 nn-imx tensorflow-lite-vx-delegate"

RM_WORK_EXCLUDE += "${PN}"

do_install(){
    install -d ${D}/opt/tflite-app
    install -m 755 ${S}/object-detection.py      --target-directory=${D}/opt/tflite-app
    install -m 755 ${S}/yolo_object_detection.py --target-directory=${D}/opt/tflite-app
    cp -rf ${S}/tflite-models ${D}/opt/tflite-app/

}

FILES:${PN} += "/opt/tflite-app"
INSANE_SKIP:${PN} += "build-deps"
