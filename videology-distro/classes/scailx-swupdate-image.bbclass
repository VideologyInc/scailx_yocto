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
    e2fsprogs-tune2fs \
    scailx-ssh-keys \
    scailx-profile \
    scailx-mounts-boot \
    scailx-mounts-storage \
"

IMAGE_FSTYPES = "squashfs"

do_fetch[depends] += "virtual/bootloader:do_deploy"

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

inherit extrausers
EXTRA_USERS_PARAMS += "usermod -a -G docker rootscailx; passwd-expire scailx; usermod -p '' root; passwd-expire root; "

do_add_scailx_ssh_keys () {
	# to allow vscode-remote
	sed -i -e '/AllowTcpForwarding/c\AllowTcpForwarding yes' ${IMAGE_ROOTFS}${sysconfdir}/ssh/sshd_config
	sed -i -e '/AllowAgentForwarding/c\AllowAgentForwarding yes' ${IMAGE_ROOTFS}${sysconfdir}/ssh/sshd_config

	# disable DNS lookups
	sed -i -e '/UseDNS/c\UseDNS no' ${IMAGE_ROOTFS}${sysconfdir}/ssh/sshd_config

	# use scailx ssh key-select script
	sed -i -e '/AuthorizedKeysCommand /c\AuthorizedKeysCommand /etc/ssh/scailx-keys.sh' ${IMAGE_ROOTFS}${sysconfdir}/ssh/sshd_config
	sed -i -e '/AuthorizedKeysCommandUser /c\AuthorizedKeysCommandUser root' ${IMAGE_ROOTFS}${sysconfdir}/ssh/sshd_config

	# allow more auth tries
	sed -i -e '/MaxAuthTries/c\MaxAuthTries 20' ${IMAGE_ROOTFS}${sysconfdir}/ssh/sshd_config
}
IMAGE_PREPROCESS_COMMAND += ";do_add_scailx_ssh_keys;"

# do_swuimage:append() {
#     import libconf, io, json
#     swd = os.path.join(d.getVar('S') ,'sw-description')
#     with open(swd, 'r') as f:
#         conf = libconf.load(f)
#     data = conf.software
#     output = os.path.join(d.getVar('WORKDIR'), 'sw-description.json')
#     json.dump(data, open(output, 'w'), indent=4)
# }

inherit swupdate-image
do_swuimage[stamp-extra-info] = "${IMAGE_LINK_NAME}"

# SWUPDATE_SIGNING : if set, the SWU is signed. There are 3 allowed values: RSA, CMS, CUSTOM. This value determines used signing mechanism.
# SWUPDATE_SIGN_TOOL : instead of using openssl, use SWUPDATE_SIGN_TOOL to sign the image. A typical use case is together with a hardware key. It is available if SWUPDATE_SIGNING is set to CUSTOM
# SWUPDATE_PRIVATE_KEY : this is the file with the private key used to sign the image using RSA mechanism. Is available if SWUPDATE_SIGNING is set to RSA.
# SWUPDATE_PASSWORD_FILE : an optional file containing the password for the private key. It is available if SWUPDATE_SIGNING is set to RSA.
# SWUPDATE_CMS_KEY : this is the file with the private key used in signing process using CMS mechanism. It is available if SWUPDATE_SIGNING is set to CMS.
# SWUPDATE_CMS_CERT : this is the file with the certificate used in signing process using CMS method. It is available if SWUPDATE_SIGNING is set to CMS.
# SWUPDATE_AES_FILE : this is the file with the AES password to encrypt artifact. A new fstype is supported by the class (type: enc). SWUPDATE_AES_FILE is generated as output from openssl to create a new key with