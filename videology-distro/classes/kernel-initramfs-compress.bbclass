# in the case arm64 or riscv, kernel uses plain Image, which is uncompressed.
# make kernel use compressed initramfs in this case.


USE_COMPRESSED_INITRD ?= "Y"

copy_initramfs() {
	echo "Copying initramfs into ./usr ..."
	# In case the directory is not created yet from the first pass compile:
	mkdir -p ${B}/usr
	# Find and use the first initramfs image archive type we find
	rm -f ${B}/usr/${INITRAMFS_IMAGE_NAME}*
    if [ "${USE_COMPRESSED_INITRD}" = "Y" ] ; then
        for img in cpio.gz cpio.lz4 cpio.lzo cpio.lzma cpio.xz cpio.zst; do
            if [ -e "${INITRAMFS_DEPLOY_DIR_IMAGE}/${INITRAMFS_IMAGE_NAME}.$img" ]; then
                cp ${INITRAMFS_DEPLOY_DIR_IMAGE}/${INITRAMFS_IMAGE_NAME}.$img ${B}/usr/.
                break
            fi
        done
    else
        if [ -e "${INITRAMFS_DEPLOY_DIR_IMAGE}/${INITRAMFS_IMAGE_NAME}.cpio" ]; then
            cp ${INITRAMFS_DEPLOY_DIR_IMAGE}/${INITRAMFS_IMAGE_NAME}.cpio ${B}/usr/.
        fi
        for img in cpio cpio.gz cpio.lz4 cpio.lzo cpio.lzma cpio.xz cpio.zst; do
            if [ -e "${INITRAMFS_DEPLOY_DIR_IMAGE}/${INITRAMFS_IMAGE_NAME}.$img" ]; then
                cp ${INITRAMFS_DEPLOY_DIR_IMAGE}/${INITRAMFS_IMAGE_NAME}.$img ${B}/usr/.
                case $img in
                *gz)
                    echo "gzip decompressing image"
                    gunzip -f ${B}/usr/${INITRAMFS_IMAGE_NAME}.$img
                    break
                    ;;
                *lz4)
                    echo "lz4 decompressing image"
                    lz4 -df ${B}/usr/${INITRAMFS_IMAGE_NAME}.$img ${B}/usr/${INITRAMFS_IMAGE_NAME}.cpio
                    break
                    ;;
                *lzo)
                    echo "lzo decompressing image"
                    lzop -df ${B}/usr/${INITRAMFS_IMAGE_NAME}.$img
                    break
                    ;;
                *lzma)
                    echo "lzma decompressing image"
                    lzma -df ${B}/usr/${INITRAMFS_IMAGE_NAME}.$img
                    break
                    ;;
                *xz)
                    echo "xz decompressing image"
                    xz -df ${B}/usr/${INITRAMFS_IMAGE_NAME}.$img
                    break
                    ;;
                *zst)
                    echo "zst decompressing image"
                    zstd -df ${B}/usr/${INITRAMFS_IMAGE_NAME}.$img
                    break
                    ;;
                esac
                break
            fi
        done
        # Verify that the above loop found a initramfs, fail otherwise
        [ -f ${B}/usr/${INITRAMFS_IMAGE_NAME}.cpio ] && echo "Finished copy of initramfs into ./usr" || die "Could not find any ${INITRAMFS_DEPLOY_DIR_IMAGE}/${INITRAMFS_IMAGE_NAME}.cpio{.gz|.lz4|.lzo|.lzma|.xz|.zst) for bundling; INITRAMFS_IMAGE_NAME might be wrong."
    fi
}

do_bundle_initramfs () {
	if [ ! -z "${INITRAMFS_IMAGE}" -a x"${INITRAMFS_IMAGE_BUNDLE}" = x1 ]; then
		echo "Creating a kernel image with a bundled initramfs..."
		copy_initramfs
		# Backing up kernel image relies on its type(regular file or symbolic link)
		tmp_path=""
		for imageType in ${KERNEL_IMAGETYPE_FOR_MAKE} ; do
			if [ -h ${KERNEL_OUTPUT_DIR}/$imageType ] ; then
				linkpath=`readlink -n ${KERNEL_OUTPUT_DIR}/$imageType`
				realpath=`readlink -fn ${KERNEL_OUTPUT_DIR}/$imageType`
				mv -f $realpath $realpath.bak
				tmp_path=$tmp_path" "$imageType"#"$linkpath"#"$realpath
			elif [ -f ${KERNEL_OUTPUT_DIR}/$imageType ]; then
				mv -f ${KERNEL_OUTPUT_DIR}/$imageType ${KERNEL_OUTPUT_DIR}/$imageType.bak
				tmp_path=$tmp_path" "$imageType"##"
			fi
		done
        if [ "${USE_COMPRESSED_INITRD}" = "Y" ] ; then
            initramfs=$(find ${B}/usr/ -name ${INITRAMFS_IMAGE_NAME}.* | head -n 1) || die "Could not find any ${INITRAMFS_DEPLOY_DIR_IMAGE}/${INITRAMFS_IMAGE_NAME}.cpio{.gz|.lz4|.lzo|.lzma|.xz|.zst) for bundling; INITRAMFS_IMAGE_NAME might be wrong."
            use_alternate_initrd=CONFIG_INITRAMFS_SOURCE=${initramfs}
        else
		    use_alternate_initrd=CONFIG_INITRAMFS_SOURCE=${B}/usr/${INITRAMFS_IMAGE_NAME}.cpio
		fi
        kernel_do_compile
		# Restoring kernel image
        bbnote "Restoring kernel image: $tmp_path"
		for tp in $tmp_path ; do
			imageType=`echo $tp|cut -d "#" -f 1`
			linkpath=`echo $tp|cut -d "#" -f 2`
			realpath=`echo $tp|cut -d "#" -f 3`
            bbnote "Restoring kernel image: $imageType $linkpath $realpath"
			if [ -n "$realpath" ]; then
				mv -f $realpath $realpath.initramfs
				mv -f $realpath.bak $realpath
				ln -sf $linkpath.initramfs ${B}/${KERNEL_OUTPUT_DIR}/$imageType.initramfs
                bbnote "Restoring kernel image: $imageType $linkpath $realpath"
            else
				mv -f ${KERNEL_OUTPUT_DIR}/$imageType ${KERNEL_OUTPUT_DIR}/$imageType.initramfs
				mv -f ${KERNEL_OUTPUT_DIR}/$imageType.bak ${KERNEL_OUTPUT_DIR}/$imageType
                bbnote "Restoring kernel image: $imageType $linkpath $realpath"
            fi
		done
	fi
}
# do_bundle_initramfs[dirs] = "${B}"

# kernel_do_transform_bundled_initramfs() {
#         # vmlinux.gz is not built by kernel
# 	if (echo "${KERNEL_IMAGETYPES}" | grep -wq "vmlinux\.gz"); then
# 		gzip -9cn < ${KERNEL_OUTPUT_DIR}/vmlinux.initramfs > ${KERNEL_OUTPUT_DIR}/vmlinux.gz.initramfs
#         fi
# }
# do_transform_bundled_initramfs[dirs] = "${B}"

# python do_devshell:prepend () {
#     os.environ["LDFLAGS"] = ''
# }

# addtask bundle_initramfs after do_install before do_deploy

# KERNEL_DEBUG_TIMESTAMPS ??= "0"

kernel_do_compile() {
	unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS MACHINE

	# setup native pkg-config variables (kconfig scripts call pkg-config directly, cannot generically be overriden to pkg-config-native)
	export PKG_CONFIG_DIR="${STAGING_DIR_NATIVE}${libdir_native}/pkgconfig"
	export PKG_CONFIG_PATH="$PKG_CONFIG_DIR:${STAGING_DATADIR_NATIVE}/pkgconfig"
	export PKG_CONFIG_LIBDIR="$PKG_CONFIG_DIR"
	export PKG_CONFIG_SYSROOT_DIR=""

	if [ "${KERNEL_DEBUG_TIMESTAMPS}" != "1" ]; then
		# kernel sources do not use do_unpack, so SOURCE_DATE_EPOCH may not
		# be set....
		if [ "${SOURCE_DATE_EPOCH}" = "" -o "${SOURCE_DATE_EPOCH}" = "0" ]; then
			# The source directory is not necessarily a git repository, so we
			# specify the git-dir to ensure that git does not query a
			# repository in any parent directory.
			SOURCE_DATE_EPOCH=`git --git-dir="${S}/.git" log -1 --pretty=%ct 2>/dev/null || echo "${REPRODUCIBLE_TIMESTAMP_ROOTFS}"`
		fi

		ts=`LC_ALL=C date -d @$SOURCE_DATE_EPOCH`
		export KBUILD_BUILD_TIMESTAMP="$ts"
		export KCONFIG_NOTIMESTAMP=1
		bbnote "KBUILD_BUILD_TIMESTAMP: $ts"
	else
		ts=`LC_ALL=C date`
		export KBUILD_BUILD_TIMESTAMP="$ts"
		bbnote "KBUILD_BUILD_TIMESTAMP: $ts"
	fi
	# The $use_alternate_initrd is only set from
	# do_bundle_initramfs() This variable is specifically for the
	# case where we are making a second pass at the kernel
	# compilation and we want to force the kernel build to use a
	# different initramfs image.  The way to do that in the kernel
	# is to specify:
	# make ...args... CONFIG_INITRAMFS_SOURCE=some_other_initramfs.cpio
	if [ "$use_alternate_initrd" = "" ] && [ "${INITRAMFS_TASK}" != "" ] ; then
		# The old style way of copying an prebuilt image and building it
		# is turned on via INTIRAMFS_TASK != ""
        if [ "${USE_COMPRESSED_INITRD}" = "Y" ] ; then
		    copy_initramfs
            initramfs=$(find ${B}/usr/ -name ${INITRAMFS_IMAGE_NAME}.* | head -n 1) || die "Could not find any ${INITRAMFS_DEPLOY_DIR_IMAGE}/${INITRAMFS_IMAGE_NAME}.cpio{.gz|.lz4|.lzo|.lzma|.xz|.zst) for bundling; INITRAMFS_IMAGE_NAME might be wrong."
		    use_alternate_initrd=CONFIG_INITRAMFS_SOURCE=${initramfs}
        else
            copy_initramfs
		    use_alternate_initrd=CONFIG_INITRAMFS_SOURCE=${B}/usr/${INITRAMFS_IMAGE_NAME}.cpio
        fi
	fi
	for typeformake in ${KERNEL_IMAGETYPE_FOR_MAKE} ; do
		oe_runmake ${typeformake} ${KERNEL_EXTRA_ARGS} $use_alternate_initrd
	done
}