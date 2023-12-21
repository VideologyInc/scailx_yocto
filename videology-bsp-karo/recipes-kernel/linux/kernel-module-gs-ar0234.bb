DESCRIPTION = "Kernel loadable module for Videology gs-ar0234"

LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

SRC_URI += "file://vid_isp_ar0234.c;subdir=${S}"
SRC_URI += "file://Makefile;subdir=${S}"

inherit module
