FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

# SRC_URI += "file://0001-dont-build-samples.patch"

# RM_WORK_EXCLUDE += "${PN}"

PACKAGECONFIG:append = " gstreamer "
