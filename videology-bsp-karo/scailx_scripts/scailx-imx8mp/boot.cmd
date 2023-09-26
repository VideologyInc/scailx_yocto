if test $reset_cause != '1'; then
	setenv fallback
fi
if test -e ${devtype} ${devnum}:${distro_bootpart} slot0; then
	setenv bootslot 0
	setenv prefix bsp0/
else
	setenv bootslot 1
	setenv prefix bsp1/
fi

setenv bootargs ${bootargs} root=/storage/bsp/${bootslot}/rootfs.squashfs ${append_bootargs} bootslot=${bootslot}

if test -e ${devtype} ${devnum}:${distro_bootpart} ${prefix}boot.env; then
	load ${devtype} ${devnum}:${distro_bootpart} ${load_addr} ${prefix}boot.env && env import ${load_addr} ${filesize}
fi

setenv initrd_addr -
load ${devtype} ${devnum}:${distro_bootpart} ${ramdisk_addr_r} ${prefix}initrd && setenv initrd_addr ${ramdisk_addr_r}:${filesize}
load ${devtype} ${devnum}:${distro_bootpart} ${kernel_addr_r} ${prefix}Image || load ${devtype} ${devnum}:${distro_bootpart} ${kernel_addr_r} ${prefix}Image-initramfs

load ${devtype} ${devnum}:${distro_bootpart} ${fdt_addr_r} ${prefix}devicetree/default.dtb
fdt addr ${fdt_addr_r}

for overlay_file in ${overlays}; do
	if load ${devtype} ${devnum}:${distro_bootpart} ${load_addr} ${prefix}devicetree/${overlay_prefix}${overlay_file}.dtbo; then
		echo "Applying kernel provided DT overlay ${overlay_prefix}${overlay_file}.dtbo"
		fdt apply ${load_addr} || setenv overlay_error "true"
	fi
done
if test "${overlay_error}" = "true"; then
	echo "Error applying DT overlays, restoring original DT"
	load ${devtype} ${devnum}:${distro_bootpart} ${fdt_addr_r} ${prefix}devicetree/default.dtb
fi
booti ${kernel_addr_r} ${initrd_addr} ${fdt_addr_r}