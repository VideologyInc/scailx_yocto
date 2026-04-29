#! /usr/bin/env python3
import glob
import json
import os
import re
from pathlib import Path

from vdlg_lvds.v4l2_detect_formats import camera_to_gst_list

"""

File:   create_cams_config.py

2026.0226.  Fixed crashing if not /dev/video? detected.
2026.0226.  Added known camera popular resolution, framerate and format list. 

2026.0302.  Added Zoom Block camera format full list (resolution, fps, formats) from Visca commands.
2026.0310.  Added more resolution formats for 3 Sony imx sensors from Framos driver repository xml files.
2026.0420.  Moved camera detection logic to separate file create_cams_config.py, added type hints and refactored code.
2026.04.28 Moved camera_dict and camera_gst_dict to separate json files

By:			Kobus (in 2025 and before), jye@videologyinc.com and mmikhaliuk@piesoft.us

"""

# Currently supports 4 camera types:
# global shutter = AR0234   => ar0234
# ZoomBlock = lvds2mipi     => zoomblock
# Boson = flir or boson     => boson
# imx series = imx          => imx

# Camera key words in device tree and its regular names
with open("/etc/default/camera_dict.json", "r") as f:
    camera_dict = json.load(f)

# Camera gst dict (high resolution, low resolution and format multiple settings tuples with 4 items each)
with open("/etc/default/camera_gst_dict.json", "r") as f:
    camera_gst_dict = json.load(f)


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
    if name == "zoomblock" or name == "boson" or name == "usb":
        cam_real_path = Path(vdev).resolve()
        info_list = camera_to_gst_list(str(cam_real_path))
    else:
        info_list = (
            camera_gst_dict[name]
            if name in camera_gst_dict
            else camera_gst_dict["ar0234"]
        )

    for info in info_list:
        gst_str = info[3]
        if len(info) == 4:
            framerate = re.search(r"framerate=(\d+)/(\d+)", gst_str).group(1)
            fps = int(framerate)
            info.append(fps)

    return info_list


def create_cam_config() -> list[tuple[str, str, int, int, int, str, str]]:
    cam_config = list[tuple[str, str, int, int, int, str, str]]()
    # iterate over cam overlays in /proc/device-tree/chosen/overlays/
    for camfile in glob.iglob("/proc/device-tree/chosen/overlays/cam*"):
        cam = os.path.basename(camfile)
        camlist = re.findall(r"cam(\d+)-(\w+)", cam)
        if len(camlist) == 0:
            continue
        idn, typ = camlist[0]
        devlist = glob.glob(f"/dev/video*csi{idn}")
        if len(devlist) == 0:
            continue
        vdev = devlist[0]

        # Get camera name and matching gst info
        name = detect_camera_by_name(cam)

        info_list = get_camera_gst(name, vdev)

        # VPU quality settings: qp above35 gives a grainy image. Below 20 the bitrate starts getting excessive.
        # Parse all resolutions and formats of the camera, may be >=2 ;-)
        for info in info_list:
            width, height, format_str, gst_str, fps = info
            cam_config.append((cam, vdev, width, height, fps, format_str, gst_str))

    # Do the same for usb camera if any. Just one now ;-)
    usb_list = glob.glob("/dev/v4l/by-path/*")
    if usb_list:
        # Find first usb camera on the list.
        for s in usb_list:
            if "usb" in s:
                vdev = str(Path(s).resolve())
                name = "usb"

                info_list = get_camera_gst(name, vdev)
                for info in info_list:
                    width, height, format_str, gst_str, fps = info
                    cam_config.append(
                        (name, vdev, width, height, fps, format_str, gst_str)
                    )
    return cam_config


def main():
    with open("/var/tmp/cams_config.json", "w") as f:
        print(f"Start get camera config from device tree path to file {f.name}")
        cam_config = create_cam_config()

        print(cam_config)

        json.dump(cam_config, f, indent=4)


if __name__ == "__main__":
    main()
