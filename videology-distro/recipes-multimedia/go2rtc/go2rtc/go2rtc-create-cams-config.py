#! /usr/bin/env python3

import yaml
import shutil
from create_cams_config import create_cam_config

"""

File:   go2rtc-create-cams-config.py

2026.0420.  Moved camera detection logic to separate file create_cams_config.py.

By:	 mmikhaliuk@piesoft.us

"""


def main():
    with open("/var/tmp/cam_config_new.yaml", "w") as f:
        print(f'Start get camera config from device tree path to file {f.name}')
        config = {"streams": {}}
        cam_config = create_cam_config()
        # cam_config = [("cam1-gs-AR0234", "/dev/video-isi-csi1", 1280, 720, 60, "default", "video/x-raw,width=1280,height=720,framerate=60/1")]
        for cam_config_item in cam_config:
            name, vdev, width, height, _fps, format_str, gst_str = cam_config_item
            config["streams"][
                f"{name}_{width}x{height}_{format_str}"
            ] = f"exec:gst-launch-1.0 -q v4l2src device={vdev} ! {gst_str} ! vpuenc_h264 qp-max=30 qp-min=20 ! fdsink"

        print(config)

        yaml.dump(config, f)

    # Copy /var/tmp/cam_config_new.yaml to /var/tmp/cam_config.yaml
    shutil.copyfile("/var/tmp/cam_config_new.yaml", "/var/tmp/cam_config.yaml")

if __name__ == "__main__":
    main()
