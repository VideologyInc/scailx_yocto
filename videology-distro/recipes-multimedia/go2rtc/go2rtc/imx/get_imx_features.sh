#!/bin/bash

if [ $# -lt 1 ]; then
  echo "Usage: $0 </dev/video id>"
  exit 1
fi

echo "Camera device is /dev/video$1"

echo Run vvget to get feature values of imx camera.

echo ========================================
echo FrameRate and Black Level Subtraction
echo
vvget $1 'FPS'
vvget $1 'BLS'

echo ============================================
echo Gamma and Denoise PreFilter
echo
vvget $1 'GAMMA ON/OFF'
vvget $1 'DPF ON/OFF'

echo ========================================================
echo Color Processing, Brightness, Contrast, Saturation, Hue
echo
vvget $1 'CPROC ON/OFF'
vvget $1 'Adjust brightness'
vvget $1 'Adjust contrast'
vvget $1 'Adjust saturation'
vvget $1 'Adjust HUE'

echo =================================================
echo AEC, SetPoint, Gain, ExposureTime, etc.
echo
vvget $1 'AEC On/Off'
vvget $1 'AEC SetPoint'
vvget $1 'AEC DampOver'
vvget $1 'AEC DampUnder'
vvget $1 'AEC Tolerance'
vvget $1 'AEC Gain'
vvget $1 'AEC ExposureTime'
vvget $1 'AEC Sensitivity'

echo ==================================
echo AWB, Gain, CCM, Offset, etc.
echo
vvget $1 'AWB On/Off'
vvget $1 'AWB Auto Ctrl item'
vvget $1 'GAIN INPUT'
vvget $1 'CCM INPUT'
vvget $1 'Offset INPUT'

echo =====================================
echo Finished




