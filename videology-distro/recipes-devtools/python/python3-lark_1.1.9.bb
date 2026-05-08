SUMMARY = "Parsing toolkit for Python"
HOMEPAGE = "https://github.com/lark-parser/lark"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=fcfbf1e2ecc0f37acbb5871aa0267500"

PYPI_PACKAGE = "lark"

SRC_URI[sha256sum] = "15fa5236490824c2c4aba0e22d2d6d823575dcaf4cdd1848e34b6ad836240fba"

inherit pypi python_setuptools_build_meta

DEPENDS += " \
    python3-setuptools-native \
    python3-wheel-native \
"
