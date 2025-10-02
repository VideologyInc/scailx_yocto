SUMMARY = "Incredibly fast JavaScript runtime, bundler, test runner, and package manager"
DESCRIPTION = "Bun is an all-in-one JavaScript runtime & toolkit designed for speed, \
               complete with a bundler, test runner, and Node.js-compatible package manager."
HOMEPAGE = "https://bun.sh"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

PV = "1.2.23"

COMPATIBLE_HOST = "(aarch64|x86_64).*-linux"
BUN_ARCH:x86-64 = "x64"
BUN_ARCH:aarch64 = "aarch64"

S = "${WORKDIR}/bun-linux-${BUN_ARCH}"

# Architecture-specific source URIs
SRC_URI:x86-64 = "https://github.com/oven-sh/bun/releases/download/bun-v${PV}/bun-linux-${BUN_ARCH}.zip;name=${BUN_ARCH}"
SRC_URI:aarch64 = "https://github.com/oven-sh/bun/releases/download/bun-v${PV}/bun-linux-${BUN_ARCH}.zip;name=${BUN_ARCH}"

SRC_URI[x64.sha256sum] = "cf0ed0a920799d576ffde4e0cae66d732bf23c2530407f26f59c7831dffe1f0e"
SRC_URI[aarch64.sha256sum] = "6a7a98c546d084a845deda62eb2a5b94a6a14a63ea81cf9186d46bf55bf910a9"

# Pre-compiled binary, skip build checks
INSANE_SKIP:${PN} += "already-stripped arch ldflags"
# No compilation needed for pre-built binary
do_compile[noexec] = "1"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${S}/bun ${D}${bindir}/bun
    ln -s -T bun ${D}${bindir}/bunx
}

FILES:${PN} = "${bindir}"