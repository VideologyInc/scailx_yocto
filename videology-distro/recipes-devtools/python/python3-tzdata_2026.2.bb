SUMMARY = "Timezone database for Python"
HOMEPAGE = "https://github.com/python/tzdata"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=fca9fd5c15a28eb874ba38577a585d48"

PYPI_PACKAGE = "tzdata"

SRC_URI[sha256sum] = "9173fde7d80d9018e02a662e168e5a2d04f87c41ea174b139fbef642eda62d10"

inherit pypi python_setuptools_build_meta

DEPENDS += " \
    python3-setuptools-native \
    python3-wheel-native \
"

# Post installation process.
# Need to copy ~/zoneinfo to /usr/share/zoninfo to make timedatectl see all zones.
do_install:append() {
    # 1. Create your target folder on the rootfs
    install -d ${D}/usr/share/

    # 2. Copy zoneinfo of the generated python site-packages folder to target /usr/share/
    cp -r ${D}${libdir}/python3.12/site-packages/tzdata/zoneinfo ${D}/usr/share/
    
}

# Add target folders and files to the package.
FILES:${PN} += "/usr/share/"

