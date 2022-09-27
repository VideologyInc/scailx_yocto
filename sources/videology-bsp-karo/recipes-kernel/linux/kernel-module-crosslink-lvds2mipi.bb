DESCRIPTION = "Kernel loadable module for Crosslink LVDS-MIPI converter"

LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

SRC_URI += "file://crosslink_lvds2mipi.c;subdir=${S}"
SRC_URI += "file://Makefile;subdir=${S}"

DEPENDS += "virtual/kernel"
inherit module
