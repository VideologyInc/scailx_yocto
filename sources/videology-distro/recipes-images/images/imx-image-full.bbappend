inherit link_default_dtb

DEFAULT_DTB = "imx8mp-qsxp-ml81-camera-crosslink.dtb"

IMAGE_INSTALL += " \
    openssh-sftp-server \
    gnupg \
    i2c-tools \
    python3-pyserial \
    gst-variable-rtsp-server \
"
