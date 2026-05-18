DESCRIPTION = "Scailx Portal and Embedded Packages"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# Tell BitBake where to find the source
S = "portal"

# Make sure scailx-ai-portal.service is properly installed in 
# 3 places - SRC_URI, install and FILES:${PN} ;-)
SRC_URI += "file://scailx-ai-portal.service"

inherit systemd

SYSTEMD_SERVICE:${PN} = "scailx-ai-portal.service"

# Add everything under ~/usr => target: /usr recursively in 3 places.
SRC_URI += "file://usr"

## ? Add empty folder ~/tmp/scailx-ai-portal to target in 2 places.

# Add folder ~/etc/scailx-ai-portal with subfolders in 3 places.
SRC_URI += "file://etc"

# Installing the application
do_install:append() {
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/scailx-ai-portal.service ${D}${systemd_system_unitdir}

    # Create ~/usr
    install -d ${D}${exec_prefix}
    # Copy folder recursively
    cp -R ${WORKDIR}/usr/* ${D}${exec_prefix}/

    # Change mod for executables and data files - folder + exec = 755, data files = 644 ;-)	
    find ${D}${exec_prefix}/bin/ -type d -exec chmod 755 {} +
    find ${D}${exec_prefix}/bin/ -type f -exec chmod 755 {} +
    
    find ${D}${exec_prefix}/lib/ -type d -exec chmod 755 {} +
    find ${D}${exec_prefix}/lib/ -type f -exec chmod 755 {} +

    find ${D}${exec_prefix}/scailx-ai-portal/ -type d -exec chmod 755 {} +
    find ${D}${exec_prefix}/scailx-ai-portal/ -type f -exec chmod 755 {} +
    
    # Create ~/tmp/scailx-ai-portal on target
    # install -d ${D}/tmp/scailx-ai-portal/
    
    # Create ~/etc on target
    install -d ${D}${sysconfdir}
    # Copy folder recursively
    cp -R ${WORKDIR}/etc/* ${D}${sysconfdir}/

    # Change mod for /etc folder and data files.	
    find ${D}${sysconfdir}/scailx-ai-portal/ -type d -exec chmod 755 {} +
    find ${D}${sysconfdir}/scailx-ai-portal/ -type f -exec chmod 644 {} +
    
    # Run touch to create 2 empty txt files
    touch ${D}${sysconfdir}/scailx-ai-portal/usenpu.txt
    touch ${D}${sysconfdir}/scailx-ai-portal/duration.txt
}

RDEPENDS:${PN} += "libmosquitto1 gstreamer1.0 glib-2.0 json-glib libcurl nnstreamer python3-core python3-pyyaml"

# Add target folders and files to the package.
FILES:${PN} += "/usr/"
# FILES:${PN} += "/tmp/"
FILES:${PN} += "/etc/"
FILES_${PN} += "${sysconfdir}/scailx-ai-portal/usenpu.txt"
FILES_${PN} += "${sysconfdir}/scailx-ai-portal/duration.txt"

