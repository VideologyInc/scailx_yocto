#!/bin/sh

[ $# -lt 1 ] && exit 0;

# split the arguments into KEY=VALUE pairs
for ARGUMENT in "$@"; do
   KEY=$(echo "$ARGUMENT" | cut -f1 -d=)
   VALUE=$(echo "$ARGUMENT" | cut -f2 -d=)
   export "$KEY"="$VALUE"
done

function get_slot
{
    for i in `cat /proc/cmdline`; do
        if [[ $i == bootslot=* ]]; then
            CURRENT_SLOT="${i: -1}"
        fi;
    done
    if [ "$CURRENT_SLOT" = "0" ]; then
        UPDATE_SLOT="1";
    else
        UPDATE_SLOT="0";
    fi
}

if [ $1 == "preinst" ]; then
    # get the current root device
    get_slot
    DISK="/dev/$(lsblk -no PKNAME /dev/disk/by-label/boot)"

    # create a symlink for the update process
    ln -sf -T "${DISK}boot${UPDATE_SLOT}" /dev/ubootdev
    ln -sf -T "/dev/disk/by-label/storage" /dev/storage
    mkdir -p /tmp/storage
    mount /dev/storage /tmp/storage
    ln -sf -T "/tmp/storage/bsp/${UPDATE_SLOT}" /tmp/update_bsp
    rm -rf /tmp/update_bsp/*
    mkdir -p /tmp/update_bsp/mounts
    rm -rf /tmp/storage/overlay/*

    ln -sf -T "/dev/disk/by-label/boot" /dev/bootdev
    mkdir -p /tmp/update_boot
    mount /dev/bootdev /tmp/update_boot
    ln -sf -T "/tmp/update_boot/bsp${UPDATE_SLOT}/" /tmp/update_boot_dir
    rm -rf /tmp/update_boot_dir/*
    # enable write on emmc boot partitions
    echo 0 > "/sys/block/mmcblk0boot${UPDATE_SLOT}/force_ro"
fi

if [ $1 == "postinst" ]; then
    get_slot
    DISK="/dev/$(lsblk -no PKNAME /dev/disk/by-label/boot)"

    # Adjust u-boot-fw-utils for eMMC on the installed rootfs
    rm -f /tmp/update_boot/slot*
    echo "$UPDATE_SLOT" > /tmp/update_boot/slot$UPDATE_SLOT
    if which fw_setenv; then
        fw_setenv bootslot $UPDATE_SLOT
        fw_setenv fdt_file default.dtb
    fi
    if [ -n "$DEFAULT_DTB" ]; then
        ln -sf -T "bsp${UPDATE_SLOT}/devicetree/${DEFAULT_DTB}" bsp${UPDATE_SLOT}/devicetree/default.dtb || cp -f "/tmp/update_boot_dir/devicetree/${DEFAULT_DTB}" /tmp/update_boot_dir/devicetree/default.dtb
        ln -sf -T "bsp${UPDATE_SLOT}/devicetree/${DEFAULT_DTB}" /tmp/update_boot/default.dtb || cp -f "/tmp/update_boot_dir/devicetree/${DEFAULT_DTB}" /tmp/update_boot/default.dtb
        ln -sf -T "bsp${UPDATE_SLOT}/boot.scr" /tmp/update_boot/boot.scr
    fi
    if which mmc; then
        # select the mmcblkXbootY partition based on UPDATE_SLOT
        # mmc bootpart enable <partition_number> <send_ack> </path/to/mmcblkX>
        mmc bootpart enable $(( $UPDATE_SLOT + 1 )) 1 ${DISK}
    fi
fi
