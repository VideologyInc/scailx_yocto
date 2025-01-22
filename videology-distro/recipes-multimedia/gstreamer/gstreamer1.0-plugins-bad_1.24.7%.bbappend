FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://0001-add-wait-for-connection-to-unixfd-from-upstream.patch"

PACKAGECONFIG[webrtcdsp] = "-Dwebrtcdsp=enabled,-Dwebrtcdsp=disabled,webrtc-audio-processing-1"
