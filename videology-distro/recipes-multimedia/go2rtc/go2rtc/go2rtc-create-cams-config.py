#! /usr/bin/env python3
import os
import yaml
import re
import glob
import tempfile

with open('/tmp/cam_config.yaml', 'w') as f:
    # print(f'{f.name}')
    config = { 'streams': {} }
    # itterate over cam overlays in /proc/device-tree/chosen/overlays/
    for camfile in glob.iglob("/proc/device-tree/chosen/overlays/cam*"):
        cam = os.path.basename(camfile)
        idn, typ = re.findall(r'cam(\d+)-(\w+)', cam)[0]
        vdev = glob.glob(f"/dev/video*csi{idn}")[0]
        # if 'crosslink' in typ:
        #     config['streams'][f'cam{idn}_1080p'] = f"exec:gst-launch-1.0 -q v4l2src device={vdev} ! video/x-raw,width=1920,height=1080,framerate=60/1 ! vpuenc_h264 qp-max=30 qp-min=20 ! fdsink"
        #     config['streams'][f'cam{idn}_720p']  = f"exec:gst-launch-1.0 -q v4l2src device={vdev} ! video/x-raw,width=1280,height=720,framerate=60/1  ! vpuenc_h264 qp-max=30 qp-min=20 ! fdsink"
        # elif 'AR0234' in typ:
        #     config['streams'][f'cam{idn}_1080p'] = f"exec:gst-launch-1.0 -q v4l2src device={vdev} ! video/x-raw,width=1920,height=1080,framerate=60/1 ! vpuenc_h264 qp-max=30 qp-min=20 ! fdsink"
        #     config['streams'][f'cam{idn}_720p']  = f"exec:gst-launch-1.0 -q v4l2src device={vdev} ! video/x-raw,width=1280,height=720,framerate=60/1  ! vpuenc_h264 qp-max=30 qp-min=20 ! fdsink"
        # elif 'os08a20' in typ:
        #     config['streams'][f'cam{idn}_1080p'] = f"exec:gst-launch-1.0 -q v4l2src device={vdev} ! video/x-raw,width=1920,height=1080,framerate=60/1 ! vpuenc_h264 qp-max=30 qp-min=20 ! fdsink"
        #     config['streams'][f'cam{idn}_720p']  = f"exec:gst-launch-1.0 -q v4l2src device={vdev} ! video/x-raw,width=1280,height=720,framerate=60/1  ! vpuenc_h264 qp-max=30 qp-min=20 ! fdsink"
        # else:
        
        # VPU quality settings: qp above35 gives a grainy image. Below 20 the bitrate starts getting excessive.
        config['streams'][f'{cam}_1080p'] = f"exec:gst-launch-1.0 -q v4l2src device={vdev} ! video/x-raw,width=1920,height=1080,framerate=60/1 ! vpuenc_h264 qp-max=30 qp-min=20 ! fdsink"
        config['streams'][f'{cam}_720p']  = f"exec:gst-launch-1.0 -q v4l2src device={vdev} ! video/x-raw,width=1280,height=720,framerate=60/1  ! vpuenc_h264 qp-max=30 qp-min=20 ! fdsink"

    yaml.dump(config, f)
