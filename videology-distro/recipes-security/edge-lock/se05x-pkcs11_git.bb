SUMMARY = "NXP SE05x PKCS11 module"
HOMEPAGE = "https://github.com/NXPPlugNTrust/se05x-pkcs11"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=3b83ef96387f14655fc854ddc3c6bd57 \
                    file://pkcs11/LICENSE.md;md5=636524f005338d77b2c83f6b111f87cd \
                    file://simw_lib/LICENSE.txt;md5=25d592776b78ef09663409514483b8c9"

SRC_URI = "gitsm://github.com/NXPPlugNTrust/se05x-pkcs11.git;protocol=https;branch=main"

# Modify these as desired
PV = "1.0+git${SRCPV}"
SRCREV = "2f0b62fcece3d850e0d50b82cf35e2db86a1aaed"

S = "${WORKDIR}/git"

inherit cmake

PACKAGECONFIG ??= "host-openssl"

PACKAGECONFIG[host-mbedtls] = "-DPTMW_HostCrypto=MBEDTLS,,mbedtls,,,host-openssl host-none"
PACKAGECONFIG[host-openssl] = "-DPTMW_HostCrypto=OPENSSL,,openssl,,,host-mbedtls host-none"
PACKAGECONFIG[host-none]    = "-DPTMW_HostCrypto=None   ,,       ,,,host-mbedtls host-openssl"

EXTRA_OECMAKE = " \
    -DPTMW_Applet=${APPLET} \
    -DPTMW_SE05X_Ver=${APPLET_VERSION} \
    -DPTMW_SE05X_Auth=${APPLET_AUTH} \
    "
APPLET         ?= "SE05X_C"
APPLET_VERSION ?= "07_02"
APPLET_AUTH    ?= "None"

SOLIBS = ".so"
FILES_SOLIBSDEV = ""