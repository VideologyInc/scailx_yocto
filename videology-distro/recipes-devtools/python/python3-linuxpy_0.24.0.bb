SUMMARY = "Python package of v4l2 controls"
HOMEPAGE = "https://github.com/tiagocoutinho/linuxpy"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=1ebbd3e34237af26da5dc08a4e440464"

PYPI_PACKAGE = "linuxpy"

SRC_URI += "file://0001-Patched-toml-for-license-text-and-setuptools-version.patch"
SRC_URI[sha256sum] = "2b44434d28d49257e859a4830267a25aa4b51b1d229f95f79d025ae7f1a5d30e"

inherit pypi python_setuptools_build_meta

DEPENDS += " \
    python3-setuptools-native \
    python3-wheel-native \
"
