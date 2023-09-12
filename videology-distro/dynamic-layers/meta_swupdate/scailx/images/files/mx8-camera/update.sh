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

function get_update_devices
{
    BOOTLOADER_PART="/dev/mmcblk0boot${UPDATE_SLOT}"
}

if [ $1 == "preinst" ]; then
    # get the current root device
    get_slot
    # get the device to be updated
    get_update_devices

    # create a symlink for the update process
    ln -sf -T "/dev/mmcblk0boot${UPDATE_SLOT}" /dev/ubootdev
    ln -sf -T "/dev/mmcblk0p3" /dev/storage
    mkdir -p /tmp/storage
    mount /dev/storage /tmp/storage
    mkdir -p /tmp/storage/bsp/0/mounts /tmp/storage/bsp/1/mounts
    rm -rf /tmp/storage/overlay/*
    ln -sf -T "/tmp/storage/bsp/${UPDATE_SLOT}" /tmp/update_bsp
    rm -rf /tmp/update_bsp/*

    if [ "$UPDATE_SLOT" = "0" ]; then
        ln -sf -T "/dev/mmcblk0p1" /dev/bootdev
    else
        ln -sf -T "/dev/mmcblk0p2" /dev/bootdev
    fi
    mkdir -p /tmp/update_boot
    mount /dev/bootdev /tmp/update_boot
    # enable write on emmc boot partitions
    echo 0 > "/sys/block/mmcblk0boot${UPDATE_SLOT}/force_ro"
fi

if [ $1 == "postinst" ]; then
    get_slot
    # Adjust u-boot-fw-utils for eMMC on the installed rootfs
    echo "$UPDATE_SLOT" > /tmp/update_boot/slot
    if which fw_setenv; then
        fw_setenv bootslot $UPDATE_SLOT
        fw_setenv mmcpart $(( $UPDATE_SLOT + 1 ))

    fi
    if which mmc; then
        # select the mmcblkXbootY partition based on UPDATE_SLOT
        # mmc bootpart enable <partition_number> <send_ack> </path/to/mmcblkX>
        mmc bootpart enable $(( $UPDATE_SLOT + 1 )) 1 /dev/mmcblk0
    fi
fi
