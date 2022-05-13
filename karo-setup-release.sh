#!/bin/bash
#
# i.MX Yocto Project Build Environment Setup Script
#
# Copyright (C) 2011-2016 Freescale Semiconductor
# Copyright 2017 NXP
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# Added changes to support
#
# Ka-Ro i.MX8 COMs inside the NXP Yocto Project Build Environment
#
# Copyright (C) 2019 Oliver Wendt <OW@KARO-electronics.de>
# Copyright (C) 2020 Lothar Waßmann <LW@KARO-electronics.de>
# Copyright (C) 2020 Markus Bauer <MB@KARO-electronics.de>
#

. sources/meta-imx/tools/setup-utils.sh

CWD=`pwd`
BASENAME="karo-setup-release.sh"
PROGNAME="setup-environment"

exit_message () {
    echo "To return to this build environment later please run:"
    echo -e "\tsource setup-environment <build_dir>"
}

usage() {
    echo "Usage: source $BASENAME [-b <build-dir>] [-h]"

    echo "Optional parameters:
* [-b <build-dir>]: Build directory, where <build-dir> is a sensible name of a
                    directory to be created.
                    If unspecified script uses 'build' as output directory.
* [-h]: help
"
}

test_builddir () {
    if [ ! -e $BUILD_DIR/conf/local.conf ]; then
	echo -e "\n ERROR - No build directory is set yet. Run the 'setup-environment' script before running this script to create $BUILD_DIR\n"
	return 1
    fi
}

clean_up() {
    unset CWD BUILD_DIR KARO_DISTRO
    unset fsl_setup_help fsl_setup_error fsl_setup_flag
    unset usage clean_up
    unset ARM_DIR META_FSL_BSP_RELEASE
    exit_message clean_up
}

layer_exists() {
    for l in $layers;do
	[ "$1" = "$l" ] && return
    done
    false
}

add_layer() {
    layer_exists && return
    layers="$layers $1 "
    echo "BBLAYERS += \"\${BSPDIR}/sources/$1\"" >> "$BUILD_DIR/conf/bblayers.conf"
}

# get command line options
OLD_OPTIND=$OPTIND
unset KARO_DISTRO

while getopts b:h: fsl_setup_flag; do
    case ${fsl_setup_flag} in
	b)
	    BUILD_DIR="$OPTARG"
	    echo "Build directory is: $BUILD_DIR"
	    ;;
	h)
	    fsl_setup_help=true
	    ;;
	*)
	    fsl_setup_error=true
	    ;;
    esac
done
shift $((OPTIND-1))

if [ $# -ne 0 ]; then
    fsl_setup_error=true
    echo "Unexpected positional parameters: '$@'" >&2
fi
OPTIND=$OLD_OPTIND
if test $fsl_setup_help; then
    usage && clean_up && return 1
elif test $fsl_setup_error; then
    clean_up && return 1
fi

if [ -z "$DISTRO" ]; then
    if [ -z "$KARO_DISTRO" ]; then
	KARO_DISTRO='karo-wayland'
    fi
    export DISTRO="$KARO_DISTRO"
else
    KARO_DISTRO="$DISTRO"
fi

if [ -z "$BUILD_DIR" ]; then
    BUILD_DIR='build-karo-nxp'
fi

if [ -z "$MACHINE" ]; then
    echo "Setting to default machine to 'tx8m-1610'"
    MACHINE='tx8m-1610'
fi

case $MACHINE in
    tx8m*)
	case $DISTRO in
	    karo*|*wayland)
		: ok
		;;
	    fsl-*)
		;;
	    *)
		echo -e "\n ERROR - $DISTRO not supported on $MACHINE\n"
#		echo -e "\n ERROR - Only Wayland distros are supported for i.MX8 or i.MX8M\n"
		return 1
		;;
	esac
	;;
    *)
	: ok
	;;
esac

layers=""

# copy new EULA into community so setup uses latest i.MX EULA
cp sources/meta-imx/EULA.txt sources/meta-freescale/EULA

# Backup CWD value as it's going to be unset by upcoming external scripts calls
CURRENT_CWD="$CWD"

# Set up the basic yocto environment
DISTRO=${KARO_DISTRO:-DISTRO} MACHINE=$MACHINE . ./$PROGNAME $BUILD_DIR

# Set CWD to a value again as it's being unset by the external scripts calls
[ -z "$CWD" ] && CWD="$CURRENT_CWD"

# Point to the current directory since the last command changed the directory to $BUILD_DIR
BUILD_DIR=.

if [ ! -e "$BUILD_DIR/conf/local.conf" ]; then
    echo -e "\n ERROR - No build directory is set yet. Run the 'setup-environment' script before running this script to create $BUILD_DIR\n"
    echo -e "\n"
    return 1
fi

# On the first script run, backup the local.conf file
# Consecutive runs, it restores the backup and changes are appended on this one.
if [ ! -e "$BUILD_DIR/conf/local.conf.org" ]; then
    cp "$BUILD_DIR/conf/local.conf" "$BUILD_DIR/conf/local.conf.org"
else
    cp "$BUILD_DIR/conf/local.conf.org" "$BUILD_DIR/conf/local.conf"
fi

if [ ! -e "$BUILD_DIR/conf/bblayers.conf.org" ]; then
    cp "$BUILD_DIR/conf/bblayers.conf" "$BUILD_DIR/conf/bblayers.conf.org"
else
    cp "$BUILD_DIR/conf/bblayers.conf.org" "$BUILD_DIR/conf/bblayers.conf"
fi

echo "" >> "$BUILD_DIR/conf/bblayers.conf"
echo "# i.MX Yocto Project Release layers" >> "$BUILD_DIR/conf/bblayers.conf"
#echo 'BBLAYERS += "${BSPDIR}/sources/meta-imx/meta-bsp"' >> "$BUILD_DIR/conf/bblayers.conf"
#echo 'BBLAYERS += "${BSPDIR}/sources/meta-imx/meta-sdk"' >> "$BUILD_DIR/conf/bblayers.conf"

add_layer meta-imx/meta-bsp
add_layer meta-imx/meta-sdk
add_layer meta-imx/meta-ml

if [ "$KARO_DISTRO" != "karo-minimal" ];then
    add_layer meta-nxp-demo-experience

    add_layer meta-openembedded/meta-gnome

    add_layer meta-qt5
    add_layer meta-python2
fi

echo "" >> "$BUILD_DIR/conf/bblayers.conf"
echo "# Ka-Ro specific layers" >> "$BUILD_DIR/conf/bblayers.conf"
add_layer meta-karo-nxp
add_layer meta-karo-distro

case $KARO_DISTRO in
    karo-custom-*)
	if [ -d "${BSPDIR}/sources/meta${KARO_DISTRO#karo-custom}" ];then
	    add_layer "meta${KARO_DISTRO#karo-custom}"
	else
	    echo "No custom layer found for distro: '$KARO_DISTRO'" >&2
	fi
	;;
esac

echo "BSPDIR='$(cd "$BSPDIR";pwd)'"
echo "BUILD_DIR='$(cd "$BUILD_DIR";pwd)'"

# Support integrating community meta-freescale instead of meta-fsl-arm
if [ -d ../sources/meta-freescale ]; then
    echo meta-freescale directory found
    # Change settings according to environment
    sed -e "s,meta-fsl-arm\s,meta-freescale ,g" -i conf/bblayers.conf
    sed -e "s,\$.BSPDIR./sources/meta-fsl-arm-extra\s,,g" -i conf/bblayers.conf
fi

cd "$BUILD_DIR"
clean_up
unset KARO_DISTRO
