#! /usr/bin/env bash

script_dir=$(dirname "$(readlink -f "$0")")

# get scailx version:
eval $(grep -m 1 "SCAILX_VERSION" "$script_dir/../.config.yaml")
echo "scailx version: $SCAILX_VERSION"

bitbake scailx-ml scailx-builder-container scailx-container-base && \
"$script_dir/sync_packages.sh" && \
bitbake scailx-builder-container -c do_populate_sdk && \

# upload containers to ghcr.io
for f in $(find deploy/images/ -name "*${SCAILX_VERSION}-oci.tar"); do
    bname=$(basename $f)
    name=${bname%"-${SCAILX_VERSION}-oci.tar"}
    echo "Uploading $name to ghcr.io with tag $SCAILX_VERSION"
    skopeo copy oci-archive:/work/scailx_yocto/build/$f docker://ghcr.io/videologyinc/${name}:$SCAILX_VERSION
done
