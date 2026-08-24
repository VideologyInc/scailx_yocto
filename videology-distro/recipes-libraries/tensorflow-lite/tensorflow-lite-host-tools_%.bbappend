# TensorFlow Lite's native-tools CMake project installs vendored FlatBuffers
# and Protobuf development files in addition to flatc and protoc.  The native
# sysroot already obtains those development files from flatbuffers-native and
# protobuf-native, so exporting them again makes eSDK dependency staging fail
# with file collisions.  The two executables are statically linked and do not
# need this development payload at runtime.
do_install:append:class-native() {
    rm -rf ${D}${includedir}
    rm -rf ${D}${libdir}
}
