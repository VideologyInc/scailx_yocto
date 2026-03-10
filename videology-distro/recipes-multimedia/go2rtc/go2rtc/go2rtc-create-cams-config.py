#! /usr/bin/env python3
import os
import yaml
import re
import glob
import tempfile
import shutil
from pathlib import Path

from vdlg_lvds.v4l2_detect_formats import camera_to_gst_list

"""

File:   go2rtc-create-cams-config.py

2026.0226.  Fixed crashing if not /dev/video? detected.
2026.0226.  Added known camera popular resolution, framerate and format list. 

2026.0302.  Added Zoom Block camera format full list (resolution, fps, formats) from Visca commands.
2026.0310.  Added more resolution formats for 3 Sony imx sensors from Framos driver repository xml files.

By:			Kobus (in 2025 and before) and jye@videologyinc.com

"""

# Currently supports 4 camera types:
# global shutter = AR0234   => ar0234
# ZoomBlock = lvds2mipi     => zoomblock
# Boson = flir or boson     => boson
# imx series = imx          => imx

# Camera key words in device tree and its regular names
camera_dict = {
    "AR0234": "ar0234",
    "lvds2mipi": "zoomblock",
    "flir": "boson",
    "imx900": "imx900",
    "imx678": "imx678",
    "imx662": "imx662",
}

# Camera gst dict (high resolution, low resolution and format multiple settings tuples with 4 items each)
camera_gst_dict = {
    "ar0234": [
        (1920, 1080, "default", "video/x-raw,width=1920,height=1080,framerate=60/1"),
        (1280, 720, "default", "video/x-raw,width=1280,height=720,framerate=60/1"),
    ],
    "zoomblock": [
        (1920, 1080, "default", "video/x-raw,width=1920,height=1080,framerate=25/1"),
        (1280, 720, "default", "video/x-raw,width=1280,height=720,framerate=25/1"),
    ],
    "boson": [
        (640, 512, "default", "video/x-raw,width=640,height=512,framerate=60/1"),
        (320, 256, "default", "video/x-raw,width=320,height=256,framerate=60/1"),
        (640, 512, "GRAY8", "video/x-raw,width=640,height=512,framerate=60/1,format=GRAY8 ! videoconvert ! video/x-raw,format=NV12"),
        (320, 256, "GRAY8", "video/x-raw,width=320,height=256,framerate=60/1,format=GRAY8 ! videoconvert ! video/x-raw,format=NV12"),
    ],
    # imx sensors use their *.xml from framos-vvcam-module repository 
    "imx900": [
        (1920, 1080, "default", "video/x-raw,width=1920,height=1080,framerate=15/1,format=YUY2"),
        (1280, 720, "default", "video/x-raw,width=1280,height=720,framerate=15/1,format=YUY2"),
        # Not supported by vpuenc_h264 (2048, 1536, "default", "video/x-raw,width=2048,height=1536,framerate=15/1,format=YUY2"),
        (1024, 768, "default", "video/x-raw,width=1024,height=768,framerate=15/1,format=YUY2"),
        (1008, 704, "default", "video/x-raw,width=1008,height=704,framerate=15/1,format=YUY2"),
    ],
    "imx678": [
        (1920, 1080, "default", "video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12"),
        (1280, 720, "default", "video/x-raw,width=1280,height=720,framerate=30/1,format=NV12"),
    ],
    "imx662": [
        (1920, 1080, "default", "video/x-raw,width=1920,height=1080,framerate=60/1,format=YUY2"),
        (1280, 720, "default", "video/x-raw,width=1280,height=720,framerate=60/1,format=YUY2"),
        (960, 540, "default", "video/x-raw,width=960,height=540,framerate=60/1,format=YUY2"),
        (640, 480, "default", "video/x-raw,width=640,height=480,framerate=60/1,format=YUY2"),
    ],
}


# Given camera name from device tree, find its matching regular name in camera_dict.
def detect_camera_by_name(cam):
    for key, val in camera_dict.items():
        if key in cam:
            return val
    # cannot find matching camera, use default global shutter ar0234.
    return "ar0234"


# Given camera name, return its width, height and gst string.
# To Do, for ZoomBlock cameras connected through LVDS2MIPI port, still need to detect and get camera gst info using gst-device-monitor ;-)
# Or with a more complex way, get its format using v4l2-ctl --list-formats-ext and "translate" to gst strings ;-)
def get_camera_gst(name, vdev):

    # For Zoom Block camera through LVDS board, use newly created info list (from Visca commands).
    if name=="zoomblock" or name=="boson":
        cam_real_path = Path(vdev).resolve()
        info_list = camera_to_gst_list(str(cam_real_path))
    else:
        info_list = (
            camera_gst_dict[name] if name in camera_gst_dict else camera_gst_dict["ar0234"]
        )

    return info_list


with open("/var/tmp/cam_config_new.yaml", "w") as f:
    print(f'Start get camera config from device tree path to file {f.name}')
    config = {"streams": {}}
    # itterate over cam overlays in /proc/device-tree/chosen/overlays/
    for camfile in glob.iglob("/proc/device-tree/chosen/overlays/cam*"):
        cam = os.path.basename(camfile)
        camlist = re.findall(r"cam(\d+)-(\w+)", cam)
        if len(camlist)==0:
            continue
        idn, typ = camlist[0]
        devlist = glob.glob(f"/dev/video*csi{idn}")
        if len(devlist)==0:
            continue
        vdev = devlist[0]

        # Get camera name and matching gst info
        name = detect_camera_by_name(cam)

        info_list = get_camera_gst(name, vdev)

        # VPU quality settings: qp above35 gives a grainy image. Below 20 the bitrate starts getting excessive.
        # Parse all resolutions and formats of the camera, may be >=2 ;-) 
        for info in info_list:
            width, height, format_str, gst_str = info
            config["streams"][f"{cam}_{width}x{height}_{format_str}"] = f"exec:gst-launch-1.0 -q v4l2src device={vdev} ! {gst_str} ! vpuenc_h264 qp-max=30 qp-min=20 ! fdsink"

    print(config)
    
    yaml.dump(config, f)

# Copy /var/tmp/cam_config_new.yaml to /var/tmp/cam_config.yaml
shutil.copyfile("/var/tmp/cam_config_new.yaml", "/var/tmp/cam_config.yaml")
