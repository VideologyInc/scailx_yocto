SUMMARY = "An image with full multimedia, Machine Learning and Basler camera support"

require karo-image-ml.bb

REQUIRED_DISTRO_FEATURES = "x11 wayland"

IMAGE_INSTALL_append = " \
        kernel-module-basler-camera \
        python3-pypylon \
        gentl-producer \
        v4l-utils \
        iperf3 \
        ethtool \
        i2c-tools \
        sclbl \
        xauth \
        imx-vpu-hantro-daemon \
        packagegroup-fsl-gstreamer1.0 \
        packagegroup-fsl-gstreamer1.0-full \
"

IMAGE_INSTALL_append_mx8mp = " \
        kernel-module-isp-vvcam \
        imx8mp-modprobe-config \
        basler-camera \
        isp-imx \
        packagegroup-imx-isp \
"

IMAGE_FEATURES_remove = "read-only-rootfs"
