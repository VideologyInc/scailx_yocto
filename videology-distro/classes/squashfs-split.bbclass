DEPENDS:append = " squashfs-tools-native "

IMAGE_TYPES += " squashsplit "

SQUASHFS_OPTS ?= "-noappend -comp xz"

SQUASH_SPLIT_DIRS ?= "lib/modules lib/firmware"
SQUASH_SPLIT_RM ?= "boot"
SQUASH_SPLIT_TMP = "${WORKDIR}/squashsplit_temp"

do_image_squashsplit[dirs] = " ${SQUASH_SPLIT_TMP} "
do_image_squashsplit[cleandirs] = " ${SQUASH_SPLIT_TMP} "

IMAGE_CMD:squashsplit(){
    cd ${IMAGE_ROOTFS}

    for d in ${SQUASH_SPLIT_DIRS}; do
        fname=$(echo $d | tr / @)
        bbnote "Creating ${IMGDEPLOYDIR}/${IMAGE_NAME}.${fname}.squashfs"
        mksquashfs "${IMAGE_ROOTFS}/$d" "${IMGDEPLOYDIR}/${IMAGE_NAME}.${fname}.squashfs" ${SQUASHFS_OPTS}
    done

    exclude=$(echo "${SQUASH_SPLIT_DIRS} ${SQUASH_SPLIT_RM}" | sed -E 's#(\S+)#\1\/\*#g')
    bbnote "Excluding ${exclude} from rootfs"
    mksquashfs ${IMAGE_ROOTFS} ${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.squashfs ${SQUASHFS_OPTS} -wildcards -e ${exclude}
}

do_image_squashsplit[imgsuffix] = "."

python() {
    sub_dirs = d.getVar('SQUASH_SPLIT_DIRS').split()
    subs = [f.replace('/', '@')+'.squashfs' for f in sub_dirs] + ['rootfs.squashfs']
    d.setVarFlag('do_image_squashsplit', 'subimages', ' '.join(subs))
}