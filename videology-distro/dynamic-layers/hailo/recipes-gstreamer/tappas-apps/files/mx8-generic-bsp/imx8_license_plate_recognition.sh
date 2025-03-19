#!/bin/bash
set -e

CURRENT_DIR="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"
${CURRENT_DIR}/resources/get_reqs.sh || echo couldnt get reqs.
${CURRENT_DIR}/resources/get_reqs.sh || echo couldnt get reqs.

q_elem="queue max-size-buffers=3 max-size-bytes=0 max-size-time=0"

function init_variables() {
    print_help_if_needed $@

    # Temporary file
    readonly FILE_LOADED_TO_CACHE_PATH=/tmp/hailo_lpr

    # Basic Directories
    readonly POSTPROCESS_DIR="/usr/lib/hailo-post-processes"
    readonly CROPPING_ALGORITHMS_DIR="$POSTPROCESS_DIR/cropping_algorithms"
    readonly RESOURCES_DIR="${CURRENT_DIR}/resources"
    readonly DEFAULT_LICENSE_PLATE_JSON_CONFIG_PATH="$RESOURCES_DIR/configs/yolov4_license_plate.json"
    readonly DEFAULT_VEHICLE_JSON_CONFIG_PATH="$RESOURCES_DIR/configs/yolov5_vehicle_detection.json"

    # Vehicle Detection Macros
    readonly VEHICLE_DETECTION_HEF="$RESOURCES_DIR/yolov5m_vehicles_no_ddr_yuy2.hef"
    readonly VEHICLE_DETECTION_POST_SO="$POSTPROCESS_DIR/libyolo_post.so"
    readonly VEHICLE_DETECTION_POST_FUNC="yolov5_vehicles_only"

    # License Plate Detection Macros
    readonly LICENSE_PLATE_DETECTION_HEF="$RESOURCES_DIR/tiny_yolov4_license_plates_yuy2.hef"
    readonly LICENSE_PLATE_DETECTION_POST_SO="$POSTPROCESS_DIR/libyolo_post.so"
    readonly LICENSE_PLATE_DETECTION_POST_FUNC="tiny_yolov4_license_plates"

    # License Plate OCR Macros
    readonly LICENSE_PLATE_OCR_HEF="$RESOURCES_DIR/lprnet_yuy2.hef"
    readonly LICENSE_PLATE_OCR_POST_SO="$POSTPROCESS_DIR/libocr_post.so"

    # Cropping Algorithm Macros
    readonly LICENSE_PLATE_CROP_SO="$CROPPING_ALGORITHMS_DIR/liblpr_croppers.so"
    readonly LICENSE_PLATE_DETECTION_CROP_FUNC="vehicles_without_ocr"
    readonly LICENSE_PLATE_OCR_CROP_FUNC="license_plate_quality_estimation"

    # Pipeline Utilities
    readonly LPR_OVERLAY="$RESOURCES_DIR/liblpr_overlay.so"
    readonly LPR_OCR_SINK="$RESOURCES_DIR/liblpr_ocrsink.so"

    cams=$(ls /dev/video-i*)
    input_source="$RESOURCES_DIR/lpr.mp4"
    print_gst_launch_only=false
    additional_parameters=""
    stats_element=""
    debug_stats_export=""
    sync_pipeline=true
    device_id_prop=""
    tee_name="context_tee"
    internal_offset=false
    pipeline_1=""
    license_plate_json_config_path=$DEFAULT_LICENSE_PLATE_JSON_CONFIG_PATH
    car_json_config_path=$DEFAULT_VEHICLE_JSON_CONFIG_PATH
}

function print_help_if_needed() {
    while test $# -gt 0; do
        if [ "$1" = "--help" ] || [ "$1" == "-h" ]; then
            print_usage
        fi

        shift
    done
}

function print_usage() {
    echo "IMX8 LPR pipeline usage:"
    echo ""
    echo "Options:"
    echo "  -h --help                  Show this help"
    echo "  --show-fps                 Print fps"
    echo "  --use-cam                  Run with camera input instead of video"
    echo "  --print-gst-launch         Print the ready gst-launch command without running it"
    echo "  --print-device-stats       Print the power and temperature measured"
    exit 0
}

function parse_args() {
    while test $# -gt 0; do
        if [ "$1" = "--print-gst-launch" ]; then
            print_gst_launch_only=true
        elif [ "$1" = "--print-device-stats" ]; then
            hailo_bus_id=$(hailortcli scan | awk '{ print $NF }' | tail -n 1)
            device_id_prop="device_id=$hailo_bus_id"
            stats_element="hailodevicestats $device_id_prop"
            debug_stats_export="GST_DEBUG=hailodevicestats:5"
        elif [ "$1" = "--show-fps" ]; then
            echo "Printing fps"
            additional_parameters="-v | grep -e hailo_display -e hailodevicestats"
        elif [ "$1" = "--use-cam" ]; then
            cam=$(set -- `ls /dev/video-i*` ; echo $1)
            if [ -n "$cam" ]; then
                echo "Using camera "
                input_source=$cam
            else
                echo "No camera found, using default video source"
            fi
        else
            echo "Received invalid argument: $1. See expected arguments below:"
            print_usage
            exit 1
        fi

        shift
    done
}

init_variables $@
parse_args $@
# if input_source container /dev/video, use v4l2src, otherwise use multifilesrc
if [[ $input_source =~ "/dev/video" ]]; then
    source_element="v4l2src device=/dev/video-isi-csi0 ! video/x-raw,format=YUY2,width=1920,height=1080,framerate=30/1 "
else
    source_element="filesrc name=src_0 location=$input_source ! qtdemux ! h264parse ! avdec_h264 ! videoconvert ! video/x-raw,format=YUY2 "
fi
internal_offset=true


function create_lp_detection_pipeline() {
    pipeline_1="$q_elem ! \
                hailocropper so-path=$LICENSE_PLATE_CROP_SO function-name=$LICENSE_PLATE_DETECTION_CROP_FUNC internal-offset=$internal_offset drop-uncropped-buffers=true name=cropper1 \
                hailoaggregator name=agg1 cropper1. !  $q_elem ! agg1. cropper1. ! $q_elem ! \
                    hailonet hef-path=$LICENSE_PLATE_DETECTION_HEF vdevice-group-id=1 scheduling-algorithm=1 ! \
                    $q_elem ! \
                    hailofilter so-path=$LICENSE_PLATE_DETECTION_POST_SO config-path=$license_plate_json_config_path function-name=$LICENSE_PLATE_DETECTION_POST_FUNC qos=false ! \
                    $q_elem ! \
                agg1. agg1. ! $q_elem ! \
                hailocropper so-path=$LICENSE_PLATE_CROP_SO function-name=$LICENSE_PLATE_OCR_CROP_FUNC internal-offset=$internal_offset drop-uncropped-buffers=true name=cropper2 \
                hailoaggregator name=agg2 cropper2. ! $q_elem ! agg2. cropper2. ! \
                    $q_elem ! \
                    hailonet hef-path=$LICENSE_PLATE_OCR_HEF vdevice-group-id=1 scheduling-algorithm=1 ! \
                    $q_elem ! \
                    hailofilter so-path=$LICENSE_PLATE_OCR_POST_SO qos=false ! \
                    $q_elem ! agg2. agg2. ! $q_elem"
}
create_lp_detection_pipeline $@

# output_elm="xvimagesink name=hailo_display sync=$sync_pipeline"
output_elm="autovideosink name=hailo_display sync=$sync_pipeline"

PIPELINE="$source_element ! $q_elem ! \
    hailonet hef-path=$VEHICLE_DETECTION_HEF vdevice-group-id=1 scheduling-algorithm=1 ! $q_elem ! \
    hailofilter so-path=$VEHICLE_DETECTION_POST_SO config-path=$car_json_config_path function-name=$VEHICLE_DETECTION_POST_FUNC qos=false ! $q_elem ! \
    hailotracker name=hailo_tracker keep-past-metadata=true kalman-dist-thr=.5 iou-thr=.6 keep-tracked-frames=2 keep-lost-frames=2 ! $q_elem ! \
    tee name=$tee_name $tee_name. ! $q_elem ! \
    videobox top=1 bottom=1 ! $q_elem ! \
    hailooverlay line-thickness=3 font-thickness=1 qos=false ! \
    hailofilter use-gst-buffer=true so-path=$LPR_OVERLAY qos=false ! \
    $output_elm \
    $tee_name. ! $pipeline_1 ! \
    hailofilter use-gst-buffer=true so-path=$LPR_OCR_SINK qos=false ! \
    fakesink sync=false async=false  ${additional_parameters}"

echo "Running License Plate Recognition"
echo ${PIPELINE}

if [ "$print_gst_launch_only" = true ]; then
    exit 0
fi

eval ${debug_stats_export} gst-launch-1.0 ${stats_element} ${PIPELINE}
