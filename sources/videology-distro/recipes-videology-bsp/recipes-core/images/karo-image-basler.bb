SUMMARY = "An image with full multimedia, Machine Learning and Basler camera support"

require karo-image-ml.bb

REQUIRED_DISTRO_FEATURES = "x11 wayland"

IMAGE_INSTALL_append = " \
        kernel-module-basler-camera \
        gentl-producer \
        v4l-utils \
        iperf3 \
        ethtool \
        i2c-tools \
        sclbl \
        nano \
        openssh-sftp \
        openssh-sftp-server \
        gstreamer1.0-rtsp-server gst-variable-rtsp-server \
        xauth \
        imx-vpu-hantro-daemon \
        packagegroup-fsl-gstreamer1.0 \
        packagegroup-fsl-gstreamer1.0-full \
"

# IMAGE_INSTALL_remove = "busybox"

IMAGE_INSTALL_append_mx8mp = " \
        kernel-module-isp-vvcam \
        imx8mp-modprobe-config \
        basler-camera \
        isp-imx \
        packagegroup-imx-isp \
"

IMAGE_FEATURES_remove = "read-only-rootfs"
