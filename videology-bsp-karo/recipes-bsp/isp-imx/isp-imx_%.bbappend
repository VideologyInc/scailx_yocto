# install vvcam headers for isp-imx userspace drivers
do_install:append() {
    install -d ${D}${includedir}/vvcam/isi
    cp -r ${S}/vvcam/* ${D}${includedir}/vvcam/
    mv    ${D}${includedir}/vvcam/common/* ${D}${includedir}/vvcam/
    cp -r ${S}/units/cameric_drv ${D}${includedir}/vvcam/
    cp -r ${S}/units/fpga ${D}${includedir}/vvcam/
    cp -r ${S}/units/hal ${D}${includedir}/vvcam/
    cp -r ${S}/units/i2c_drv ${D}${includedir}/vvcam/
    cp -r ${S}/units/oslayer ${D}${includedir}/vvcam/
    cp -r ${S}/units/isi/include/* ${D}${includedir}/vvcam/isi/
    cp -r ${S}/units/isi/include_priv/* ${D}${includedir}/vvcam/isi/
}
