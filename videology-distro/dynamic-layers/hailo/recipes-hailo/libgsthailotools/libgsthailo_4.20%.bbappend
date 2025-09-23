FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://0003-fix-cmake-protobuf.patch"

DEPENDS += "protobuf-native protobuf"
EXTRA_OECMAKE += "-DProtobuf_PROTOC_EXECUTABLE=${STAGING_BINDIR_NATIVE}/protoc"


