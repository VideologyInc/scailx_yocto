
# basler patchset
SRC_URI_append = "\
    file://0001-make-sensor-controls-available-in-mx6s_capture.patch  \
    file://0002-added-SRGGB8-format-support.patch  \
    file://0003-rxhs-settle-and-send-level-value-is-now-optinal-conf.patch  \
    file://0004-increase-max-video-memory-from-64-to-128MB.patch  \
    file://0005-implement-cache-invalidation-for-MMAP.patch  \
    file://0006-handle-subdev-unbind.patch  \
    file://0007-media-csi-Add-support-for-RAW10.patch  \
    file://0008-media-mxc-mipi_csi-Add-UYVY8_2X8.patch  \
    file://0009-media-mxc-capture-Enable-two_8bit_sensor_mode-based-.patch  \
    file://0010-media-mxc-Enable-dual-pixel-mode-for-YUV422.patch  \
    file://0011-arm64-dts-imx8mm-Add-DTS-for-Basler-camera.patch  \
    file://0012-arm64-dts-imx8mm-ddr4-Add-DTS-for-Basler-camera.patch  \
    file://0013-arm64-dts-imx8mq-Add-DTS-for-Basler-camera.patch  \
    file://0014-arm64-dts-imx8mm-Add-DT-variants-using-one-CSI-lane.patch  \
    file://0015-mxc-capture-Add-debug-event-interrupt-counters.patch  \
    file://0016-mxc-Fix-MASK_OPTION-in-CSICR18-to-avoid-loosing-firs.patch  \
    file://0017-Reformatted-Makefile.patch  \
    file://0018-Merged-PR-9649-Enable-imx8mq-evk-camera-trigger.patch  \
    file://0019-Merged-PR-9974-Added-highspeed-device-tree-for-imx8m.patch  \
    file://0020-media-mxc-change-mx6s_fmt.bpp-to-bits-per-pixel.patch  \
    file://0021-media-mxc-add-support-for-YUV420-NV12.patch  \
    file://0022-Add-basler-camera-on-CSI1-via-ISI1.patch  \
    file://0023-Add-basler-camera-on-CSI0-via-ISI0-chained.patch  \
    file://0024-imx8mp-isi-add-dual-isi-dtb.-fix-max-link-frequencie.patch  \
    file://0026-LF-2670-02-media-isi-core-disable-gate-clock-for-isi.patch  \
"

