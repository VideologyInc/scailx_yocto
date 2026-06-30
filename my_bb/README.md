Videology Yocto Customized Recipes
=======================

# Copy and Build (For incremental build only)

- nnstreamer mqtt update

Copy current updated "nnstreamer_2.4.0.bb" to overwrite 

~/meta-imx/meta-imx-ml/recipes-nnstreamer/nnstreamer/

# First time build

- kas shell and bitbake nnstreamer
- Now subfolder ~/meta-imx/meta-imx-ml/recipes-nnstreamer/nnstreamer/ exists, 
copy current new bb file in this folder to target folder as shown above.
- bitbake -c cleansstate nnstreamer
- bitbake -c cleansstate scailx-ml
- bitbake nnstreamer
- Build scailx-ml as usual.

----------------------------------------------

Rocketsan's linux-imx driver fix.

devtool modify linux-imx

Then copy imx-isi-cap.c to override 

~./build/workspace/sources/linux-imx/drivers/staging/media/imx/imx8-isi-cap.c
