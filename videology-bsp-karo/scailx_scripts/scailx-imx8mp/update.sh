#!/bin/sh

[ $# -lt 1 ] && exit 0;

# split the arguments into KEY=VALUE pairs
for ARGUMENT in "$@"; do
   KEY=$(echo "$ARGUMENT" | cut -f1 -d=)
   VALUE=$(echo "$ARGUMENT" | cut -f2 -d=)
   export "$KEY"="$VALUE"
done

function get_slot {
    for i in `cat /proc/cmdline`; do
        if [[ $i == bootslot=* ]]; then
            CURRENT_SLOT="${i: -1}"
        fi;
    done
    # if recovery flag is set, use the other slot
    if [ "$CURRENT_SLOT" = "1" ]; then
        export UPDATE_SLOT="0";
        [ -f /boot/recovery ] && export UPDATE_SLOT="1";
    else
        export UPDATE_SLOT="1";
        [ -f /boot/recovery ] && export UPDATE_SLOT="0";
    fi
}

format_disk() {
    # get the primary disk
    for i in `cat /proc/cmdline`; do
        [[ $i == primary-disk=* ]] && dev="${i: -1}"
    done
    [ -b "$dev" ] || dev=$(findfs LABEL=boot || findfs LABEL=root || findfs LABEL=rootfs); dev=$(echo $dev | sed -r 's/p?[0-9]*$//')
    [ -b "$dev" ] || (echo "not a block device: $dev" && return 1)
    # unmount everything on the disk
    umount -f /boot;     umount -f /rootfs;     umount -f /storage;

    # get hw sector size for disk
    # bzs=$(cat /sys/block/$(basename "$dev")/queue/hw_sector_size); size=$((450 * 1024 * 1024 / bzs))
	# partition device:
	# gpt label, 512M boot, rest is storage
    sfdisk -w always -W always "$dev" <<EOF
label: gpt

name=boot,  size=880640,    type=21686148-6449-6E6F-744E-656564454649, bootable
name=storage,                  type=0FC63DAF-8483-4772-8E79-3D69D8477DE4
EOF

	udevadm settle

    mkfs.ext4 "$(findfs PARTLABEL=boot)" -L boot
	mkfs.ext4 "$(findfs PARTLABEL=storage)" -L storage
}

if [ $1 == "preinst" ]; then
    # get the current root device
    get_slot
    DISK=$(findfs LABEL=boot | sed -r 's/p?[0-9]*$//')
    [ -b $DISK ] || exit 1

    # create a symlink for the update process
    mkdir -p /tmp/storage
    mount /dev/disk/by-label/storage /tmp/storage || (umount -f /tmp/storage; mount /dev/disk/by-label/storage /tmp/storage)
    rm -rf /tmp/storage/bsp/$UPDATE_SLOT/*
    mv -f /tmp/storage/initrd_log_fatal /tmp/storage/old_initrd_log_fatal || echo "no initrd fatal log to save"
    mkdir -p /tmp/storage/bsp/$UPDATE_SLOT/mounts
    [ -d /tmp/storage/overlay/upper/etc ] && (mkdir -p /tmp/storage/config/persist/; cp -fr -t /tmp/storage/config/persist/ /tmp/storage/overlay/upper/etc)
    # [ -d /tmp/storage/overlay/upper/etc/ssh ] && (mkdir -p /tmp/storage/config/persist/etc/; cp -fr -t /tmp/storage/config/persist/etc/ /tmp/storage/overlay/upper/etc/ssh)
    # [ -f /tmp/storage/overlay/upper/etc/hostname ] && (mkdir -p /tmp/storage/config/persist/etc/; cp -fr -t /tmp/storage/config/persist/etc/ /tmp/storage/overlay/upper/etc/hostname)
    rm -rf /tmp/storage/overlay/backup*
    mv -f /tmp/storage/overlay/upper /tmp/storage/overlay/backup
    rm -rf /tmp/storage/overlay/work
    # remove files in upper that are already in persist
    persist_files=$(find /tmp/storage/config/persist/ -type f | sed -r 's/\/tmp\/storage\/config\/persist\///')
    for f in $persist_files; do rm -f /tmp/storage/overlay/backup/$f; done
    # tarzip files in upper relative to upper
    # tar -C /tmp/storage/overlay/upper -czf /tmp/storage/backup.tgz .

    mkdir -p /tmp/update_boot
    mount /dev/disk/by-label/boot /tmp/update_boot || (umount -f /dev/disk/by-label/boot; mount /dev/disk/by-label/boot /tmp/update_boot)
    rm -rf /tmp/update_boot/bsp${UPDATE_SLOT}/*
    # enable write on emmc boot partitions
    echo 0 > "/sys/block/mmcblk0boot${UPDATE_SLOT}/force_ro"
    sync; umount -f /tmp/storage; umount -f /tmp/update_boot;
fi

if [ $1 == "postinst" ]; then
    get_slot
    DISK=$(findfs PARTLABEL=boot | sed -r 's/p?[0-9]*$//')

    # create partlabels for the devices created with the update in case they don't exist
    e2label /dev/disk/by-partlabel/storage 'storage'
    e2label /dev/disk/by-partlabel/boot    'boot'

    # Adjust u-boot-fw-utils for eMMC on the installed rootfs
    mount /dev/disk/by-partlabel/boot /tmp/update_boot
    rm -f /tmp/update_boot/slot*
    echo "$UPDATE_SLOT" > /tmp/update_boot/slot$UPDATE_SLOT
    echo "$UPDATE_SLOT" > /tmp/update_boot/updated$UPDATE_SLOT
    if which fw_setenv; then
        fw_setenv bootslot $UPDATE_SLOT
        fw_setenv fdt_file default.dtb
    fi
    if [ -n "$DEFAULT_DTB" ]; then
        ln -sf -T "${DEFAULT_DTB}" /tmp/update_boot/bsp${UPDATE_SLOT}/devicetree/default.dtb || cp -f "/tmp/update_boot/bsp${UPDATE_SLOT}/devicetree/${DEFAULT_DTB}" /tmp/update_boot/bsp${UPDATE_SLOT}/devicetree/default.dtb
        ln -sf -T "bsp${UPDATE_SLOT}/devicetree/${DEFAULT_DTB}" /tmp/update_boot/default.dtb || cp -f "/tmp/update_boot/bsp${UPDATE_SLOT}/devicetree/${DEFAULT_DTB}" /tmp/update_boot/default.dtb
        ln -sf -T "bsp${UPDATE_SLOT}/boot.scr" /tmp/update_boot/boot.scr
    fi
    if which mmc; then
        # select the mmcblkXbootY partition based on UPDATE_SLOT
        # mmc bootpart enable <partition_number> <send_ack> </path/to/mmcblkX>
        mmc bootpart enable $(( $UPDATE_SLOT + 1 )) 1 ${DISK}
    fi
fi
