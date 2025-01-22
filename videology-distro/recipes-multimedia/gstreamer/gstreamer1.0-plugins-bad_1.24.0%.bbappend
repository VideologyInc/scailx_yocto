FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
PACKAGECONFIG[webrtcdsp] = "-Dwebrtcdsp=enabled,-Dwebrtcdsp=disabled,webrtc-audio-processing-1"

SRC_URI += " file://0001-update-unixfd-with-to-wait-for-connection.patch "
