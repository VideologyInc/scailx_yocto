FILESEXTRAPATHS:append := ":${THISDIR}/files:"

# INFO: its been metioned that firmware 4.19 renamed hailonet to synchailonet, and on some platforms the new hailonet doesnt work.
# https://community.hailo.ai/t/unexpected-hailo-driver-fail-36-errors-occuring-with-hailort-4-19/6495

# SRC_URI += "file://0001-replace-hailonet-with-synchailonet.patch;patchdir=${WORKDIR}/git"

# dont populate licenseplate demo as theyolov4-tiny model doesnt work.
# INSTALL_LPR = "false"

SRC_URI += "file://imx8_detection.sh"
SRC_URI += "file://imx8_face_detection_and_landmarks.sh"
SRC_URI += "file://imx8_depth_estimation.sh"
SRC_URI += "file://imx8_license_plate_recognition.sh"
SRC_URI += "file://scailx_hailo_lpr.py"

do_install:append() {
    install -m 0755 ${WORKDIR}/imx8_detection.sh ${D}${INSTALL_DIR}/detection/
    install -m 0755 ${WORKDIR}/imx8_depth_estimation.sh ${D}${INSTALL_DIR}/depth_estimation/
    install -m 0755 ${WORKDIR}/imx8_face_detection_and_landmarks.sh ${D}${INSTALL_DIR}/cascading_networks/
    install -m 0755 ${WORKDIR}/imx8_license_plate_recognition.sh ${D}${INSTALL_DIR}/license_plate_recognition/
    install -m 0755 ${WORKDIR}/scailx_hailo_lpr.py ${D}${INSTALL_DIR}/license_plate_recognition/

    # remove unmodified scripts that dont work
    rm -rf  ${D}${INSTALL_DIR}/cascading_networks
    rm -rf  ${D}${INSTALL_DIR}/depth_estimation/depth_estimation.sh
    rm -rf  ${D}${INSTALL_DIR}/detection/detection.sh
    rm -rf  ${D}${INSTALL_DIR}/license_plate_recognition/license_plate_recognition.sh
}