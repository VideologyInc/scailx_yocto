#!/bin/bash

# from bitbake build-sysroots -c build_native_sysroot && bitbake build-sysroots -c build_target_sysroot

# if [ $# -ne 1 ]; then
#     echo "Usage: $0 <recipe-name>"
#     exit 1
# fi

# RECIPE_NAME=$1

bitbake meta-ide-support
# bitbake -c populate_sysroot $RECIPE_NAME
# or any other target or native item that the application developer would need
bitbake build-sysroots -c build_native_sysroot 
bitbake build-sysroots -c build_target_sysroot
