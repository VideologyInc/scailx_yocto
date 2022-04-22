# Copyright 2020 NXP
DESCRIPTION = "Kernel loadable module for ISP"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${WORKDIR}/vvcam/LICENSE;md5=64381a6ea83b48c39fe524c85f65fb44"


SRC_URI = "file://kernel-module-isp-vvcam/kernel-module-isp_${PV}.tar.gz"
S = "${WORKDIR}/vvcam/v4l2"


inherit module


