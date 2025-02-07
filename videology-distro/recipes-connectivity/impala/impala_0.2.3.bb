SUMMARY = "TUI for managing wifi on Linux"
DESCRIPTION = "A vhost-user backend that emulates a VirtIO random number \
    generator device"
HOMEPAGE = "https://github.com/pythops/impala"
LICENSE = "GPL-3.0-only"
# LIC_FILES_CHKSUM = "file://LICENSE;md5=1ebbd3e34237af26da5dc08a4e440464"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-3.0-only;md5=c79ff39f19dfec6d293b95dea7b07891"

inherit cargo
inherit cargo-update-recipe-crates
include impala-crates.inc

SRC_URI += "crate://crates.io/impala/0.2.3"
SRC_URI[impala-0.2.3.sha256sum] = "1d6ec454a3454db73060f3a127d670d6a354e855fde3ad731693333063055537"

INSANE_SKIP:${PN} += "already-stripped"

RDEPENDS:${PN} += "iwd"