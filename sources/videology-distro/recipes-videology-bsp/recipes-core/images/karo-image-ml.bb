SUMMARY = "An image with full multimedia and Machine Learning support"

require recipes-core/images/karo-image-weston.bb

GOOGLE_CORAL_PKGS = " \
        libedgetpu \
"

OPENCV_PKGS_imxgpu = " \
        opencv-apps \
        opencv-samples \
        python3-opencv \
        python3-pygobject \
"

IMAGE_INSTALL_append = " \
        ${GOOGLE_CORAL_PKGS} \
        ${OPENCV_PKGS} \
        packagegroup-fsl-tools-gpu \
        packagegroup-fsl-tools-gpu-external \
        packagegroup-imx-ml \
        python3-pip \
        tzdata \
"

TOOLCHAIN_TARGET_TASK_append = " \
        tensorflow-lite-staticdev \
"
