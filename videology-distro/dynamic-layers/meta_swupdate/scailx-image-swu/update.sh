#!/bin/sh

if [ $# -lt 1 ]; then
    exit 0;
fi

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

function get_update_devices
{
    BOOTLOADER_PART="/dev/mmcblk0boot${UPDATE_SLOT}"
    BOOT_PART
}

if [ $1 == "preinst" ]; then
    # get the current root device
    get_slot
    # get the device to be updated
    get_update_devices

    # create a symlink for the update process
    mkdir -p /bsp/0/ /bsp/1/
    ln -sf "/bsp/${UPDATE_SLOT}" /update_bsp
    ln -sf "/dev/mmcblk0boot${UPDATE_SLOT}" /dev/ubootdev
    ln -sf "/dev/mmcblk0p3" /dev/storage
    ln -sf "/dev/mmcblk0p1" /dev/bootdev
    # enable write on emmc boot partitions
    echo 0 > "/sys/block/mmcblk0boot${UPDATE_SLOT}/force_ro"
fi

if [ $1 == "postinst" ]; then
    get_slot

    # Adjust u-boot-fw-utils for eMMC on the installed rootfs
    echo "$UPDATE_SLOT" > /boot/slot
    fw_setenv bootslot $UPDATE_SLOT
fi
