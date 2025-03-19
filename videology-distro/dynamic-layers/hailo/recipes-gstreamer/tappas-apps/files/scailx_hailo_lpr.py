#!/usr/bin/env python3
# lp_demo.py - License Plate Recognition Demo

import os
import sys
import glob
import argparse
import subprocess
import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib, GObject
Gst.init(None)

class LPRPipeline:
    def __init__(self):
        # Initialize GStreamer
        Gst.init(None)

        self.pipe = None
        self.loop = GLib.MainLoop()

        # Paths and settings
        self.current_dir = os.path.dirname(os.path.abspath(__file__))
        self.resources_dir = os.path.join(self.current_dir, "resources")

        # Temporary file
        self.file_loaded_to_cache_path = "/tmp/hailo_lpr"

        # Basic Directories
        self.postprocess_dir = "/usr/lib/hailo-post-processes"
        self.cropping_algorithms_dir = f"{self.postprocess_dir}/cropping_algorithms"

        # Config files
        self.default_license_plate_json_config_path = f"{self.resources_dir}/configs/yolov4_license_plate.json"
        self.default_vehicle_json_config_path = f"{self.resources_dir}/configs/yolov5_vehicle_detection.json"

        # Vehicle Detection
        self.vehicle_detection_hef = f"{self.resources_dir}/yolov5m_vehicles_no_ddr_yuy2.hef"
        self.vehicle_detection_post_so = f"{self.postprocess_dir}/libyolo_post.so"
        self.vehicle_detection_post_func = "yolov5_vehicles_only"

        # License Plate Detection
        self.license_plate_detection_hef = f"{self.resources_dir}/tiny_yolov4_license_plates_yuy2.hef"
        self.license_plate_detection_post_so = f"{self.postprocess_dir}/libyolo_post.so"
        self.license_plate_detection_post_func = "tiny_yolov4_license_plates"

        # License Plate OCR
        self.license_plate_ocr_hef = f"{self.resources_dir}/lprnet_yuy2.hef"
        self.license_plate_ocr_post_so = f"{self.postprocess_dir}/libocr_post.so"

        # Cropping Algorithm
        self.license_plate_crop_so = f"{self.cropping_algorithms_dir}/liblpr_croppers.so"
        self.license_plate_detection_crop_func = "vehicles_without_ocr"
        self.license_plate_ocr_crop_func = "license_plate_quality_estimation"

        # Pipeline Utilities
        self.lpr_overlay = f"{self.resources_dir}/liblpr_overlay.so"
        self.lpr_ocr_sink = f"{self.resources_dir}/liblpr_ocrsink.so"
        # self.q_elm = "queue max-size-buffers=5 max-size-bytes=0 max-size-time=0"
        self.q_elm = "queue max-size-buffers=4 max-size-bytes=0 max-size-time=0"

        # Default settings
        self.input_source = os.path.realpath(f"{self.resources_dir}/lpr.mp4")
        cams = glob.glob("/dev/video-i*")
        if cams:
            self.input_source = cams[0]
        self.stats_element = ""
        self.sync_pipeline = True
        self.device_id_prop = ""
        self.tee_name = "context_tee"
        self.internal_offset = False
        self.license_plate_json_config_path = self.default_license_plate_json_config_path
        self.car_json_config_path = self.default_vehicle_json_config_path

    def parse_args(self):
        parser = argparse.ArgumentParser(description='IMX8 LPR pipeline')
        parser.add_argument('--print-device-stats', action='store_true', help='Print the power and temperature measured')
        parser.add_argument('input', nargs='?', default=None, help="input to LPR. Either V4L2 device or MP4 file to use. Example: /dev/video0 or /path/to/video.mp4. MUST BE 1080p30 EXACTLY.")

        args = parser.parse_args()

        if args.print_device_stats:
            # Get Hailo device bus ID
            hailo_bus_id = self._get_hailo_bus_id()
            self.device_id_prop = f"device_id={hailo_bus_id}"
            self.stats_element = f"hailodevicestats {self.device_id_prop}"
            os.environ["GST_DEBUG"] = "hailodevicestats:5"
        if args.input:
            self.input_source = args.input
        return args

    def _get_hailo_bus_id(self):
        try:
            result = subprocess.check_output("hailortcli scan", shell=True).decode('utf-8')
            lines = result.strip().split('\n')
            if lines:
                last_line = lines[-1]
                bus_id = last_line.split()[-1]
                return bus_id
        except Exception as e:
            print(f"Error getting Hailo bus ID: {str(e)}")
        return ""

    def create_source_element(self):
        if "/dev/video" in self.input_source:
            # Camera source
            return f"v4l2src device={self.input_source} ! video/x-raw,format=YUY2,width=1920,height=1080,framerate=30/1 "
        elif ".mp4" in self.input_source:
            # File source
            self.internal_offset = True
            return f"filesrc name=src_0 location={self.input_source} ! qtdemux ! h264parse ! avdec_h264 ! videoconvert ! video/x-raw,format=YUY2 "
        else:
            raise ValueError(f"Unsupported input source: {self.input_source}")

    def create_lp_detection_pipeline(self):
        pipeline_1 = f"""{self.q_elm} ! \
            hailocropper so-path={self.license_plate_crop_so} function-name={self.license_plate_detection_crop_func} internal-offset={str(self.internal_offset).lower()} drop-uncropped-buffers=true name=cropper1 \
            hailoaggregator name=agg1 \
            cropper1. ! \
            {self.q_elm} ! agg1. cropper1. ! {self.q_elm} ! \
            hailonet hef-path={self.license_plate_detection_hef} vdevice-group-id=1 scheduling-algorithm=1 ! \
            {self.q_elm} ! \
            hailofilter so-path={self.license_plate_detection_post_so} config-path={self.license_plate_json_config_path} function-name={self.license_plate_detection_post_func} qos=false ! \
            {self.q_elm} ! agg1. agg1. ! {self.q_elm} ! \
            hailocropper so-path={self.license_plate_crop_so} function-name={self.license_plate_ocr_crop_func} internal-offset={str(self.internal_offset).lower()} drop-uncropped-buffers=true name=cropper2 \
            hailoaggregator name=agg2 cropper2. ! {self.q_elm} ! \
            agg2. cropper2. ! {self.q_elm} ! \
            hailonet hef-path={self.license_plate_ocr_hef} vdevice-group-id=1 scheduling-algorithm=1 ! \
            {self.q_elm} ! \
            hailofilter so-path={self.license_plate_ocr_post_so} qos=false ! \
            {self.q_elm} ! agg2. agg2. ! {self.q_elm}"""
        return pipeline_1

    def build_pipeline(self, source_pipeline=None, sink_element=None):
        source_element = source_pipeline if source_pipeline else self.create_source_element()
        lp_detection_pipeline = self.create_lp_detection_pipeline()

        # sinkelm = sink_element if sink_element else f"xvimagesink name=hailo_display sync={str(self.sync_pipeline).lower()}"
        sinkelm = f"autovideosink name=hailo_display "

        pipeline_str = f""" \
            {self.stats_element} \
            {source_element} ! \
            {self.q_elm} ! \
            hailonet hef-path={self.vehicle_detection_hef} vdevice-group-id=1 scheduling-algorithm=1 ! \
            {self.q_elm} ! \
            hailofilter so-path={self.vehicle_detection_post_so} config-path={self.car_json_config_path} function-name={self.vehicle_detection_post_func} qos=false ! \
            {self.q_elm} ! \
            hailotracker name=hailo_tracker keep-past-metadata=true kalman-dist-thr=.5 iou-thr=.6 keep-tracked-frames=2 keep-lost-frames=2 ! \
            {self.q_elm} ! \
            tee name={self.tee_name} \
            {self.tee_name}. ! {self.q_elm} ! \
            videobox top=1 bottom=1 ! \
            {self.q_elm} ! \
            hailooverlay line-thickness=2 font-thickness=1 qos=false ! \
            hailofilter use-gst-buffer=true so-path={self.lpr_overlay} qos=false ! \
            {sinkelm}
            {self.tee_name}. ! \
            {lp_detection_pipeline} ! \
            hailofilter use-gst-buffer=true so-path={self.lpr_ocr_sink} qos=false ! \
            fakesink sync=false async=false"""

        return ' '.join([p for p in pipeline_str.split() if p])

    def run(self):
        # Parse command line arguments
        self.parse_args()
        # Build the pipeline
        pipeline_str = self.build_pipeline()
        print("Running License Plate Recognition")

        # Create and start pipeline
        self.pipe = Gst.parse_launch(pipeline_str)
        self.pipe.set_state(Gst.State.PLAYING)

        try:
            self.loop.run()

        except KeyboardInterrupt:
            print("Pipeline stopped by user")
            print("Pipeline was: \n", pipeline_str)
        finally:
            # Cleanup
            self.loop.quit()

    def check_and_install_resources(self):
        # Try to run the get_reqs.sh script similar to the bash script
        reqs_script = os.path.join(self.resources_dir, "get_reqs.sh")
        if os.path.exists(reqs_script):
            try:
                subprocess.run(f"{reqs_script}", shell=True)
            except Exception:
                print("Couldn't get reqs.")


if __name__ == "__main__":
    pipeline = LPRPipeline()
    pipeline.check_and_install_resources()
    pipeline.run()
