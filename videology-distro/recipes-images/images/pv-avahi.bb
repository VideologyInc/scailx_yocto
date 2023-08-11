SUMMARY = "Pantahub avahi image"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

IMAGE_FSTYPES = "container oci pvrexportit"
inherit image
inherit image-oci

# PREFERRED_PROVIDER_virtual/kernel = "linux-dummy"
IMAGE_FEATURES = ""
IMAGE_LINGUAS = ""
NO_RECOMMENDATIONS = "1"

IMAGE_INSTALL = " \
        avahi-daemon \
        avahi-services-ssh \
        base-files \
        base-passwd \
        netbase \
        busybox \
"

# Workaround /var/volatile for now
ROOTFS_POSTPROCESS_COMMAND += "rootfs_fixup_var_volatile ; "
rootfs_fixup_var_volatile () {
    install -m 1777 -d ${IMAGE_ROOTFS}/${localstatedir}/volatile/tmp
    install -m 755 -d ${IMAGE_ROOTFS}/${localstatedir}/volatile/log
}

IMAGE_CONTAINER_NO_DUMMY = "1"

inherit image-pvrexport

PANTAHUB_API = "api.pantahub.com"

PANTAHUB_USER ?= "pantavisor-apps"

do_image_pvr_app:append() {
    sed -i '/lxc.tty.min/d' ${PN}/lxc.container.conf
    pvr add
    pvr commit
}

BB_STRICT_CHECKSUM = "0"

