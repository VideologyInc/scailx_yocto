DESCRIPTION = "TAPPAS ARM applications recipe, \
               the recipe copies the app script, hef files and media to /opt/hailo/apps \
               the apps hefs and media urls are taken from files/download_reqs.txt"

PV_PARSED = "${@ '${PV}'.replace('.0', '')}"
SRC_URI = "git://git@github.com/hailo-ai/tappas.git;protocol=https;branch=master"
SRC_URI += "file://download_reqs.txt"

S = "${WORKDIR}/git/core/hailo"

SRCREV = "7e36ff78bb99c32700efe5dad3820271210c3f58"

LICENSE = "LGPLv2.1"
LIC_FILES_CHKSUM += "file://../../LICENSE;md5=4fbd65380cdd255951079008b364516c"

inherit hailotools-base

# Setting meson build target as 'apps'
TAPPAS_BUILD_TARGET = "apps"

DEPENDS += " gstreamer1.0 gstreamer1.0-plugins-base cxxopts rapidjson"
RDEPENDS:${PN} += " bash libgsthailotools"

LPR_APP_NAME ?= "license_plate_recognition"

OPENCV_UTIL ?= "libhailo_cv_singleton.so"
GST_IMAGES_UTIL ?= "libhailo_gst_image.so"

INSTALL_DIR ?= "/opt/hailo/apps"
ROOTFS_APPS_DIR ?= "${D}${INSTALL_DIR}"

APPS_DIR_PREFIX ?= "${WORKDIR}/git/apps/"
IMX8_DIR ?= "${APPS_DIR_PREFIX}/h8/gstreamer/imx8/"
HAILO15_DIR ?= "${APPS_DIR_PREFIX}/h15/gstreamer/"

REQS_IMX8_FILE ?= "${FILE_DIRNAME}/files/download_reqs_imx8.txt"
REQS_HAILO15_FILE ?= "${FILE_DIRNAME}/files/download_reqs_hailo15.txt"

ARM_APPS_DIR ?= ""
REQS_FILE ?= "download_reqs.txt"
ARM_APPS_DIR:mx8-nxp-bsp = "${IMX8_DIR}"
ARM_APPS_DIR:hailo15 = "${HAILO15_DIR}"
DEPENDS:hailo15:append = " libmedialib-api xtensor "

IS_H15 = "${@ 'true' if 'hailo15' in d.getVar('MACHINE') else 'false'}"
INSTALL_LPR ?= "true"

CURRENT_APP_NAME = ""
CURRENT_REQ_FILE = ""

# meson configuration
EXTRA_OEMESON += " \
        -Dapps_install_dir='${INSTALL_DIR}' \
        -Dinstall_lpr='${INSTALL_LPR}' \
        -Dlibcxxopts='${STAGING_INCDIR}/cxxopts' \
        -Dlibrapidjson='${STAGING_INCDIR}/rapidjson' \
        "

python do_split_reqs() {
    workdir = d.getVar('WORKDIR')
    with open(os.path.join(workdir, d.getVar('REQS_FILE')), "r") as req_file:
        app_reqs = {}
        for line in req_file:
            # iterate over download_reqs.txt, parse each line
            url, app_path, md5sum = line.strip().split(' -> ')
            req_file = url.split('/')[-1]
            app_name = app_path.split('/')[-1]
            if app_name in app_reqs:
                app_reqs[app_name].append((req_file, url, app_path, md5sum))
            else:
                app_reqs[app_name] = [(req_file, url, app_path, md5sum)]
    with open(os.path.join(workdir, 'apps'), 'w') as f:
        f.write(" ".join(app_reqs.keys()))
    for app, files in app_reqs.items():
        files_list = ' '.join([f[1] for f in files])
        with open(os.path.join(workdir, f"get_reqs_{app}.sh"), "w") as f:
            f.write("#!/bin/bash\n\n")
            f.write("cd $(dirname \"$0\")\n\n")
            f.write(f"resource_files=\"{files_list}\"")
            f.write("\n\n")
            f.write("for file in $resource_files; do\n")
            f.write("    [ -f $(basename $file) ] || wget $file\n")
            f.write("done\n")
}
addtask split_reqs after do_configure before do_compile

do_install:append() {
    # Meson installs shared objects in apps target,
    # we remove it from the rootfs to prevent duplication with libgsthailotools
    rm -rf ${D}${libdir}/libgsthailometa*
    rm -rf ${D}${includedir}/gsthailometa
    rm -rf ${D}${libdir}/pkgconfig/gsthailometa.pc
    rm -rf ${D}${libdir}/libhailo_tracker*

    if [ '${IS_H15}' = 'true' ]; then
        install -d ${ROOTFS_APPS_DIR}/encoder_pipelines_new_api/configs/
        install -m 0755 ${S}/apps/hailo15/encoder_pipelines_new_api/*.json ${ROOTFS_APPS_DIR}/encoder_pipelines_new_api/configs/
    fi

    for app in $(cat "${WORKDIR}/apps"); do
        install -d ${ROOTFS_APPS_DIR}/${app}
        install -d ${ROOTFS_APPS_DIR}/${app}/resources

        # copy the required file into the app path under resources directory
        install -m 0755 "${WORKDIR}/get_reqs_${app}.sh" ${ROOTFS_APPS_DIR}/${app}/resources/get_reqs.sh
        # copy the app shell script into the app path
        if ls ${ARM_APPS_DIR}/${app}/*.sh >/dev/null 2>&1; then
            install -m 0755 ${ARM_APPS_DIR}/${app}/*.sh ${ROOTFS_APPS_DIR}/${app}
        else
            bbnote ".sh file not found, skipping install"
        fi
        if [ -d "${ARM_APPS_DIR}/${app}/configs" ]; then
            install -d ${ROOTFS_APPS_DIR}/${app}/resources/configs
            install -m 0755 ${ARM_APPS_DIR}/${app}/configs/* ${ROOTFS_APPS_DIR}/${app}/resources/configs
        fi
    done
}

create_bl1_image(){
    dd conv=notrunc bs=1 if=${D}/firmware/bl1_1.bin of=${D}/firmware/bl1.bin seek=0
    dd conv=notrunc bs=1 if=${D}/firmware/bl1_provisioning_bundle.bin of=${D}/firmware/bl1.bin seek=40960
}
python do_patch_scripts() {
    # Add a line to the script to get resources
    from glob import glob
    apps_dir = d.getVar('ROOTFS_APPS_DIR')
    app_scripts = glob(f"{apps_dir}/*/*.sh")
    for script in set(app_scripts):
        # find line in script with CURRENT_DIR=... in and add text after it
        bb.note(f"Adding get_reqs.sh to {script}")
        with open(script, "r") as f:
            lines = f.readlines()
        with open(script, "w") as f:
            for line in lines:
                f.write(line)
                if line.startswith("CURRENT_DIR="):
                    bb.note(f"Adding get_reqs.sh to {script}")
                    f.write("${CURRENT_DIR}/resources/get_reqs.sh || echo couldnt get reqs.\n")
}
do_install[postfuncs] += "do_patch_scripts"

FILES:${PN} += " ${INSTALL_DIR}/* ${libdir}/${OPENCV_UTIL}.${PV} ${libdir}/${GST_IMAGES_UTIL}.${PV}"
FILES:${PN}-lib += "${libdir}/${OPENCV_UTIL}.${PV} ${libdir}/${GST_IMAGES_UTIL}.${PV}"
RDEPENDS:${PN}-staticdev = ""
RDEPENDS:${PN}-dev = ""
RDEPENDS:${PN}-dbg = ""
