# image class to include kernel module build artifacts.

IMAGE_INSTALL += " \
    packagegroup-core-buildessential        \
    libgcc  \
    kernel-dev      \
    kernel-modules  \
    kernel-devsrc   \
"
