
require recipes-fsl/packagegroup/packagegroup-imx-ml.bb

ML_PKGS = "virtual/opencl-icd opencl-headers"
ML_PKGS:append:mx8-nxp-bsp = " \
    tensorflow-lite \
    torchvision \
"
ML_PKGS:append:mx9-nxp-bsp = " \
    tensorflow-lite \
    torchvision \
"

# todo: add PACKAGECONFIG[opencl] to opencv

