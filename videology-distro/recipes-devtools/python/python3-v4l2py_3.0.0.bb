SUMMARY = "Python package of v4l2 controls"
HOMEPAGE = "https://github.com/tiagocoutinho/v4l2py"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=1ebbd3e34237af26da5dc08a4e440464"

PYPI_PACKAGE = "v4l2py"

SRC_URI[sha256sum] = "7e83c02f7393da883c791b9b7ba3dd11163b42d15e68dc09b3e3d99a6d75b7a4"

inherit pypi python_setuptools_build_meta

DEPENDS += " \
    python3-setuptools-native \
    python3-wheel-native \
"
