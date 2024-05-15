do_image_docker_archive[depends] += "skopeo-native:do_populate_sysroot"
IMAGE_TYPEDEP:docker-archive = "oci"

IMAGE_CMD:docker-archive () {
    cd ${IMGDEPLOYDIR}
    image_name="${IMAGE_BASENAME}-${OCI_IMAGE_TAG}-oci"
    skopeo --insecure-policy copy "oci:$image_name:${OCI_IMAGE_TAG}" "docker-archive:${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.docker-archive:${IMAGE_BASENAME}-${OCI_IMAGE_ARCH}:${OCI_IMAGE_TAG}"
}
