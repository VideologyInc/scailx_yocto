
require recipes-fsl/packagegroup/packagegroup-imx-ml.bb

ML_PKGS += "virtual/opencl-icd opencl-headers"

ML_PKGS:remove:mx8-nxp-bsp = " \
    onnxruntime-tests \
    deepview-rt-examples \
"

ML_NNSTREAMER_PKGS:remove:mx8mp-nxp-bsp = "nnstreamer-unittest"

# todo: add PACKAGECONFIG[opencl] to opencv

