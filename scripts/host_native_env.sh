#! /bin/bash -e

source ~/.bashrc

# Kobus: use local cache if available
if [ -d "/work/share" ]; then
	export DL_DIR="/work/share/downloads"
    export SSTATE_DIR="/work/share/sstate-cache"
    # export PERSISTENT_DIR="/work/share/yocto_persistent"
    export BB_ENV_PASSTHROUGH_ADDITIONS="DL_DIR SSTATE_DIR"
fi

source .venv/bin/activate && echo "using python $(python --version)" || echo "No virtualenv found. using local"

source "${PWD}/poky/oe-init-build-env" "$1"
