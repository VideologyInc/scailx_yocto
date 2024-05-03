if test $reset_cause != '1'; then setenv fallback 1; fi
if test -e ${devtype} ${devnum}:${distro_bootpart} slot0; then setenv bootslot 0; else setenv bootslot 1 ; fi

setenv load_boot_env ' \
if test -e ${devtype} ${devnum}:${distro_bootpart} bsp${bootslot}/boot.env; then
	load ${devtype} ${devnum}:${distro_bootpart} ${load_addr} bsp${bootslot}/boot.env && env import ${load_addr} ${filesize}
fi'

setenv load_kernel_options '
if load ${devtype} ${devnum}:${distro_bootpart} ${ramdisk_addr_r} bsp${bootslot}/initrd; then
	setenv initrd_addr ${ramdisk_addr_r}:${filesize} &&
	load ${devtype} ${devnum}:${distro_bootpart} ${kernel_addr_r} bsp${bootslot}/Image &&
	setenv kernel_loaded 'true';
else
	setenv initrd_addr -;
	load ${devtype} ${devnum}:${distro_bootpart} ${kernel_addr_r} bsp${bootslot}/Image-initramfs;
fi'

setenv load_dtb_options 'load ${devtype} ${devnum}:${distro_bootpart} ${fdt_addr_r} bsp${bootslot}/devicetree/default.dtb && fdt addr ${fdt_addr_r}'
setenv load_dtb_overlay_options ' \
setenv overlay_error "false"; \
for overlay_file in ${overlays}; do
	if load ${devtype} ${devnum}:${distro_bootpart} ${load_addr} bsp${bootslot}/devicetree/${overlay_prefix}${overlay_file}.dtbo; then
		echo "Applying kernel provided DT overlay ${overlay_prefix}${overlay_file}.dtbo";
		fdt apply ${load_addr} || setenv overlay_error "true"
	fi
done
if test "${overlay_error}" = "true"; then
	echo "Error applying DT overlays, restoring original DT";
	load ${devtype} ${devnum}:${distro_bootpart} ${fdt_addr_r} bsp${bootslot}/devicetree/default.dtb && fdt addr ${fdt_addr_r};
else
	true;
fi'

if test $fallback != 1; then
	run load_boot_env;
	run load_kernel_options && run load_dtb_options &&
	setenv bootargs "root=/storage/bsp/${bootslot}/rootfs.squashfs ${append_bootargs} bootslot=${bootslot}" &&
	run load_dtb_overlay_options && booti ${kernel_addr_r} ${initrd_addr} ${fdt_addr_r};
fi

if test $bootslot = 0; then setenv bootslot 1; else setenv bootslot 0; fi
echo "load or boot failed. Fallback to bootslot ${bootslot}"
run load_boot_env;
run load_kernel_options && run load_dtb_options &&
setenv bootargs "root=/storage/bsp/${bootslot}/rootfs.squashfs ${append_bootargs} bootslot=${bootslot} fallback=1" &&
run load_dtb_overlay_options && booti ${kernel_addr_r} ${initrd_addr} ${fdt_addr_r};

echo "load or boot failed. Fallback to U-Boot shell"
