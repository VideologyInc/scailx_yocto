LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit core-image
RM_WORK_EXCLUDE += "${PN}"
inherit extra-dirs
EXTRA_ROOTFS_DIRS += "storage ${nonarch_libdir}/modules"
inherit scailx-uboot-env

FILESEXTRAPATHS:prepend := "${SCAILX_SCRIPTS_DIRS}:"
SRC_URI += "file://update.sh"

IMAGE_FEATURES += " \
"
# overlayfs-etc
# read-only-rootfs

CORE_IMAGE_EXTRA_INSTALL += " \
    libubootenv-bin \
	u-boot-default-env \
	mmc-utils \
    volatile-binds \
    swupdate \
	swupdate-www \
    swupdate-config \
    scailx-mounts-boot \
    scailx-mounts-storage \
"

IMAGE_FSTYPES = "squashfs"


# IMAGE_DEPENDS: list of Yocto images that contains a root filesystem
# it will be ensured they are built before creating swupdate image
IMAGE_DEPENDS += "virtual/kernel virtual/bootloader virtual/dtb scailx-boot-script"

# SWUPDATE_IMAGES: list of images that will be part of the compound image
# the list can have any binaries - images must be in the DEPLOY directory
SWUPDATE_IMAGES += " \
    imx-boot-karo \
    devicetrees \
    Image-initramfs \
    boot \
    uboot-env \
"

# Images can have multiple formats - define which image must be
# taken to be put in the compound image
SWUPDATE_IMAGES_FSTYPES[Image-initramfs] = ".bin"
SWUPDATE_IMAGES_FSTYPES[devicetrees] = ".tgz"
SWUPDATE_IMAGES_FSTYPES[boot] = ".scr"
SWUPDATE_IMAGES_FSTYPES[uboot-env] = ".txt"

python () {
    linkname = d.getVar('IMAGE_LINK_NAME')
    d.setVarFlag("SWUPDATE_IMAGES_FSTYPES", linkname, ".squashfs")
}

do_fetch:append() {
    s = d.getVar('DEPLOY_DIR_IMAGE')
    output_env_file(d, os.path.join(s,'uboot-env.txt'))
}

inherit swupdate-image
do_swuimage[stamp-extra-info] = "${IMAGE_LINK_NAME}"

# SWUPDATE_SIGNING : if set, the SWU is signed. There are 3 allowed values: RSA, CMS, CUSTOM. This value determines used signing mechanism.
# SWUPDATE_SIGN_TOOL : instead of using openssl, use SWUPDATE_SIGN_TOOL to sign the image. A typical use case is together with a hardware key. It is available if SWUPDATE_SIGNING is set to CUSTOM
# SWUPDATE_PRIVATE_KEY : this is the file with the private key used to sign the image using RSA mechanism. Is available if SWUPDATE_SIGNING is set to RSA.
# SWUPDATE_PASSWORD_FILE : an optional file containing the password for the private key. It is available if SWUPDATE_SIGNING is set to RSA.
# SWUPDATE_CMS_KEY : this is the file with the private key used in signing process using CMS mechanism. It is available if SWUPDATE_SIGNING is set to CMS.
# SWUPDATE_CMS_CERT : this is the file with the certificate used in signing process using CMS method. It is available if SWUPDATE_SIGNING is set to CMS.
# SWUPDATE_AES_FILE : this is the file with the AES password to encrypt artifact. A new fstype is supported by the class (type: enc). SWUPDATE_AES_FILE is generated as output from openssl to create a new key with