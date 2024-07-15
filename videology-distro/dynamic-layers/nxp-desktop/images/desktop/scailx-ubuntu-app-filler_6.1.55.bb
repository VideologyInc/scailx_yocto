require recipes-fsl/images/imx-image-desktop.bb

IMAGE_FSTYPES = "tar.gz"

IMAGE_INSTALL += " \
    signing-keys \
    scailx-tflite-mobilenet-demo \
"