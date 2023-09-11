DEPENDS:append = " squashfs-tools-native "

# Create a squashfs image of the kernel modules
kernel_do_deploy:append() {
    if (grep -q -i -e '^CONFIG_MODULES=y$' .config); then
        rm -f $deployDir/modules-${MODULE_TARBALL_NAME}.squashfs
        mksquashfs ${D}${root_prefix}/lib/modules $deployDir/modules-${MODULE_TARBALL_NAME}.squashfs -all-root
        if [ -n "${MODULE_TARBALL_LINK_NAME}" ] ; then
            ln -sf modules-${MODULE_TARBALL_NAME}.squashfs $deployDir/modules-${MODULE_TARBALL_LINK_NAME}.squashfs
        fi
    fi
}
