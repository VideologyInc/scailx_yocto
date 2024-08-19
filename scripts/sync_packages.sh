#! /usr/bin/env bash

# needs to be rnu from build folder, in yocto context.

bitbake package-index
# upload packages for all, armv8a, armv8a-mx8mp, scailx_imx8mp
rclone sync deploy/deb/all/ scailx-storage:scailx-ppa/scailx/6.6-nanbield/deb/all/ --progress --transfers 64 --checkers 64 --checksum
rclone sync deploy/deb/armv8a/ scailx-storage:scailx-ppa/scailx/6.6-nanbield/deb/armv8a/ --progress --transfers 64 --checkers 64 --checksum
rclone sync deploy/deb/armv8a-mx8mp/ scailx-storage:scailx-ppa/scailx/6.6-nanbield/deb/armv8a-mx8mp --progress --transfers 64 --checkers 64 --checksum
rclone sync deploy/deb/scailx_imx8mp/ scailx-storage:scailx-ppa/scailx/6.6-nanbield/deb/scailx_imx8mp --progress --transfers 64 --checkers 64 --checksum