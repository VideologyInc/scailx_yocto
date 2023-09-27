#!/bin/sh

### luks partition type: CA7D7CCB-63ED-4C53-861C-1742536059CC

dev="$1"
# get hw sector size for disk
# bzs=$(cat /sys/block/$(basename "$dev")/queue/hw_sector_size)
# size=$((450 * 1024 * 1024 / bzs))
# partition device:
# gpt
#  512M boot
#  rest is storage

[ -b "$dev" ] || echo "not a block device: $dev" && return 1

sfdisk -w always -W always "$dev" <<EOF
label: gpt

name=boot,  size=880640,    type=21686148-6449-6E6F-744E-656564454649, bootable
name=storage,               type=0FC63DAF-8483-4772-8E79-3D69D8477DE4
EOF

udevadm settle

mkfs.ext4 "${dev}1" -L boot
mkfs.ext4 "${dev}2" -L storage

	# trap c_bind EXIT

	# # defaults to 1MiB align
	# cryptsetup luksFormat "${dev}4" -c aes-xts-plain64
	# cryptsetup luksOpen "${dev}4" "$bdev"

	# pvcreate -ff "/dev/mapper/$bdev"
	# vgcreate "$name" "/dev/mapper/$bdev"
	# lvcreate -n root -L 7G "$name"

	# mkfs.ext4 "/dev/$name/root"
}

# c_bind() {
# 	lvchange -an "$name" || true
# 	cryptsetup close "$bdev" || true
# }

# c_mount() {
# 	umount "/mnt/$name/boot/EFI" || true
# 	umount "/mnt/$name/boot" || true
# 	umount "/mnt/$name" || true
# 	rmdir  "/mnt/$name" || true
# 	c_bind
# }

# a_bind() {
# 	trap c_bind EXIT
# 	cryptsetup luksOpen "${dev}4" "$bdev"
# 	vgchange -ay "$name"
# }
