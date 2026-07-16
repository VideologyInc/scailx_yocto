#!/bin/bash

echo umount /mnt/yocto

sudo umount /mnt/yocto

echo Use qemu to add vdi file

sudo qemu-nbd -c /dev/nbd0 yocto_backup.vdi

echo mount /dev/nbd0 to /mnt/yocto

sudo mount /dev/nbd0 /mnt/yocto

echo list contents of /mnt/yocto

ls -l /mnt/yocto


