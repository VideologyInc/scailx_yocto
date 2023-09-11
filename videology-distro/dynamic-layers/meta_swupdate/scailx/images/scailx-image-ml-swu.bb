DESCRIPTION = "Example compound image for Variscite boards"
SECTION = ""

# Note: sw-description is mandatory
SRC_URI = " \
    file://sw-description \
    file://update.sh \
"

inherit swupdate
DEFAULT_DTB = "scailx_karo_crosslink1.dtb"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SWU_ROOTFS_IMAGE = "scailx-image-swupdate-ml"

# IMAGE_DEPENDS: list of Yocto images that contains a root filesystem
# it will be ensured they are built before creating swupdate image
IMAGE_DEPENDS += "${SWU_ROOTFS_IMAGE} virtual/kernel virtual/bootloader"

# SWUPDATE_IMAGES: list of images that will be part of the compound image
# the list can have any binaries - images must be in the DEPLOY directory
SWUPDATE_IMAGES = " \
    ${SWU_ROOTFS_IMAGE} \
    imx-boot-karo \
    devicetrees \
    modules \
    Image-initramfs \
"

# Images can have multiple formats - define which image must be
# taken to be put in the compound image
SWUPDATE_IMAGES_FSTYPES[Image-initramfs] = ".bin"
SWUPDATE_IMAGES_FSTYPES[devicetrees] = ".tgz"
SWUPDATE_IMAGES_FSTYPES[modules] = ".squashfs"
python () {
    d.setVarFlag("SWUPDATE_IMAGES_FSTYPES", d.getVar('SWU_ROOTFS_IMAGE'), ".squashfs")
}

# SWUPDATE_SIGNING : if set, the SWU is signed. There are 3 allowed values: RSA, CMS, CUSTOM. This value determines used signing mechanism.
# SWUPDATE_SIGN_TOOL : instead of using openssl, use SWUPDATE_SIGN_TOOL to sign the image. A typical use case is together with a hardware key. It is available if SWUPDATE_SIGNING is set to CUSTOM
# SWUPDATE_PRIVATE_KEY : this is the file with the private key used to sign the image using RSA mechanism. Is available if SWUPDATE_SIGNING is set to RSA.
# SWUPDATE_PASSWORD_FILE : an optional file containing the password for the private key. It is available if SWUPDATE_SIGNING is set to RSA.
# SWUPDATE_CMS_KEY : this is the file with the private key used in signing process using CMS mechanism. It is available if SWUPDATE_SIGNING is set to CMS.
# SWUPDATE_CMS_CERT : this is the file with the certificate used in signing process using CMS method. It is available if SWUPDATE_SIGNING is set to CMS.
# SWUPDATE_AES_FILE : this is the file with the AES password to encrypt artifact. A new fstype is supported by the class (type: enc). SWUPDATE_AES_FILE is generated as output from openssl to create a new key with