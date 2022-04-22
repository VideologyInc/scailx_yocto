SUMMARY = "Basler pylon Camera Software Suite"
DESCRIPTION = "A software package comprised of an easy-to-use SDK and tools that you can use to operate any Basler camera."
LICENSE = "pylon & LGPLv3 & LGPLv2.1 & BSD-3-Clause & BSD-2-Clause & bzip2 & Libpng & Zlib & GenICam-1.1 & NI & xxHash & Apache-2.0"
LIC_FILES_CHKSUM = "file://share/pylon/licenses/License.html;md5=6a23d6496f15e590f32b3d3954297683 \
                    file://share/pylon/licenses/pylon_Third-Party_Licenses.html;md5=3ed299ffc665be3cebd634d25a41de80"

PR = "r0"
PYLON_VERSION = "_6.2.0.21487_aarch64"

SRC_URI = "file://${PYLON_TAR_FILENAME};subdir=${S}"

RDEPENDS_${PN} = "fontconfig freetype libsm libdrm libxcb-glx"

FILES_${PN} += "/opt/ /usr/bin/pylon"

INSANE_SKIP_${PN} += "already-stripped"
INSANE_SKIP_${PN} += "dev-so"
INSANE_SKIP_${PN} += "dev-elf"

# Work around for RDEPENDS to avoid 'no libGL.so.1()(64bit) provider found' problem
SKIP_FILEDEPS_${PN} = "1"

# Without this option yocto checks every .so included in pylon and treats it as possible RPROVIDES.
# This lead to spurious dependencies of gstreamer packages on pylon.
EXCLUDE_FROM_SHLIBS = "1"

# We want the complete pylon inside the sysroot to be able to build against it
SYSROOT_DIRS = "/opt"

PACKAGES_${PN} = "${PN}"

PYLON_TAR_FILENAME = "pylon${PYLON_VERSION}.tar.gz"

do_install() {
    # Copy the contents of the source folder (pylon folder) to opt folder
    install -d "${D}/opt/pylon"
    cp -dR --preserve=mode,links,timestamps --no-preserve=ownership "${S}"/* "${D}/opt/pylon/"

    # Create a script file which sets automatically the GENICAM_GENTL64_PATH env variable before it starts the pylon viewer.
    install -d "${D}/usr/bin"
    printf '#!/bin/sh\nGENICAM_GENTL64_PATH=/opt/dart-bcon-mipi/lib exec /opt/pylon/bin/pylonviewer "${@}"\n' >"${D}"/usr/bin/pylon
    chmod 755 "${D}"/usr/bin/pylon
}
