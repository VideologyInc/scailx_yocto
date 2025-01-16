FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-include-missing-cstdint.patch;patchdir=../.."

inherit hailotools-base
