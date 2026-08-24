#
# Based on examples from Scott Murray (Building Container Images with
# OpenEmbedded and the Yocto Project) ELCe 2018
#
SUMMARY = "Basic container image"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

IMAGE_FSTYPES = "container docker-archive.xz"
# get rid of 'rootfs' tag
IMAGE_NAME_SUFFIX ?= ""

inherit core-image
inherit image-oci

OCI_IMAGE_ENTRYPOINT = "bash"
OCI_IMAGE_TAG = "${SCAILX_VERSION}"

IMAGE_FEATURES = " package-management "

IMAGE_LINGUAS = ""
NO_RECOMMENDATIONS = "1"

# RM_WORK_EXCLUDE += " ${PN}"

OVERLAYFS_QA_SKIP[storage] = "mount-configured"
MACHINE_FEATURES = ""

IMAGE_INSTALL = " \
        base-files \
        base-passwd \
        netbase \
        ${CONTAINER_SHELL} \
        ${INSTALL_VSCODE} \
        ca-certificates \
        wget \
        bash \
        sed \
        readline \
        grep \
"

INSTALL_VSCODE = " \
	openssh-sftp-server \
    packagegroup-core-ssh-openssh \
    curl \
    libstdc++ \
"

# If the following is configured in local.conf (or the distro):
#      PACKAGE_EXTRA_ARCHS:append = " container-dummy-provides"
#
# it has been explicitly # indicated that we don't want or need a shell, so we'll
# add the dummy provides.
#
# This is required, since there are postinstall scripts in base-files and base-passwd
# that reference /bin/sh and we'll get a rootfs error if there's no shell or no dummy
# provider.
CONTAINER_SHELL ?= "${@bb.utils.contains('PACKAGE_EXTRA_ARCHS', 'container-dummy-provides', 'container-dummy-provides', 'busybox', d)}"

# Allow build with or without a specific kernel
IMAGE_CONTAINER_NO_DUMMY = "1"

# Workaround /var/volatile for now
ROOTFS_POSTPROCESS_COMMAND += "rootfs_fixup_var_volatile ; "
rootfs_fixup_var_volatile () {
    install -m 1777 -d ${IMAGE_ROOTFS}/${localstatedir}/volatile/tmp
    install -m 755 -d ${IMAGE_ROOTFS}/${localstatedir}/volatile/log
}

do_add_scailx_version () {
    echo "${SCAILX_VERSION}" > ${IMAGE_ROOTFS}${sysconfdir}/scailx-version
}
IMAGE_PREPROCESS_COMMAND += ";do_add_scailx_version;"

TARGET_HOSTNAME ?= "scailx-container"

ROOTFS_POSTPROCESS_COMMAND:append = " hostname_change; "
hostname_change () {
    echo "${TARGET_HOSTNAME}" > ${IMAGE_ROOTFS}${sysconfdir}/hostname
}

ROOTFS_POSTPROCESS_COMMAND:append = " termcolor; "
termcolor () {
    echo "TERM=xterm-256color" >> ${IMAGE_ROOTFS}${sysconfdir}/environment
}
