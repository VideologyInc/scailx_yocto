# fix for usrmerge
do_install() {
	install -d ${D}${nonarch_base_libdir}/firmware/hailo
	install -m 0755 ${FW_PATH} ${D}${nonarch_base_libdir}/firmware/hailo/hailo8_fw.bin
}

FILES:${PN} += "${nonarch_base_libdir}/firmware ${nonarch_base_libdir}/firmware/hailo/hailo8_fw*"
