DESCRIPTION = "Basler Demo Image"
LICENSE = "MIT"

require recipes-fsl/images/fsl-image-validation-imx.bb

IMAGE_INSTALL_append = " kernel-module-basler-camera \
                         pylon \
                         python3-pypylon \
                         gentl-producer \
                         xauth"

# Install license files in the image
COPY_LIC_DIRS = "1"
COPY_LIC_MANIFEST = "1"

deduplicate_licenses () {
    tmpdir=`mktemp -d`
    trap "rm -rf ${tmpdir}" exit
    mkdir -p "${tmpdir}/.src"

    bbnote "Compactifying licenses tree"

    cd "${IMAGE_ROOTFS}/usr/share"

    find 'common-licenses' \( -type f -o -type l \) -print0 | xargs -r0 sha256sum |
    {
        while read hash path; do
            if [ ! -f "${tmpdir}/.src/${hash}" ]; then
                cp -L "${path}" "${tmpdir}/.src/${hash}"
            fi

            mkdir -p `dirname "${tmpdir}/${path}"`
            ln "${tmpdir}/.src/${hash}" "${tmpdir}/${path}"
        done
    }

    rm -rf './common-licenses'
    cp -at . "${tmpdir}/common-licenses"
}

ROOTFS_POSTPROCESS_COMMAND += "deduplicate_licenses;"
