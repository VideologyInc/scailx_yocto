DESCRIPTION = "Scailx Portal and Embedded Packages"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# Tell BitBake where to find the source
S = "portal"

# Sample local source file to compile: Reference the local file
# SRC_URI = "file://helloworld.c"

# Sample do_compile loop: Compiling the application
# do_compile() {
#    ${CC} ${CFLAGS} ${LDFLAGS} helloworld.c -o helloworld
# }

# Make sure gstd.service is properly installed in 3 places ;-)
SRC_URI += "file://scailx-ai-portal.service"

inherit systemd

SYSTEMD_SERVICE:${PN} = "scailx-ai-portal.service"

# Add everything under ~/usr => target: /usr recursively.

SRC_URI += "file://usr"

# Installing the application
do_install:append() {
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/scailx-ai-portal.service ${D}${systemd_system_unitdir}

    # Create ~/usr
    install -d ${D}${exec_prefix}
    # Copy folder recursively
    cp -R ${WORKDIR}/usr/* ${D}${exec_prefix}/

    # Change mod for executables and data files.	
    find ${D}${exec_prefix}/bin/ -type d -exec chmod 755 {} +
    find ${D}${exec_prefix}/lib/ -type f -exec chmod 644 {} +
    find ${D}${exec_prefix}/scailx-ai-portal/ -type f -exec chmod 644 {} +

}

RDEPENDS:${PN} += "libmosquitto1 gstreamer1.0 glib-2.0 libcurl nnstreamer python3-core python3-pyyaml"

FILES:${PN} += "/usr/"

