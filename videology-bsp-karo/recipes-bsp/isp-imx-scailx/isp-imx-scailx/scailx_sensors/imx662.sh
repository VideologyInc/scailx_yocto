#!/bin/bash

# IMX662 Sensor Configuration Script
# This script contains IMX662-specific functions and configurations

# Sensor information
SENSOR_TYPE="imx662"
SENSOR_NAME="imx662"
DRIVER_FILE="imx662.drv"
MODE_FILE="IMX662_MODES.txt"

# Write default mode file for IMX662
write_default_modes() {
    echo "Writing IMX662 default modes..."
    echo -n "" > IMX662_MODES.txt
    echo "[mode.0]" >> IMX662_MODES.txt
    echo "xml = \"IMX662_Basic_1920x1080.xml\"" >> IMX662_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_IMX662_Basic_1920x1080.json\"" >> IMX662_MODES.txt
    echo "[mode.1]" >> IMX662_MODES.txt
    echo "xml = \"IMX662_Basic_1280x720.xml\"" >> IMX662_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_IMX662_Basic_1280x720.json\"" >> IMX662_MODES.txt
    echo "[mode.2]" >> IMX662_MODES.txt
    echo "xml = \"IMX662_Basic_960x540.xml\"" >> IMX662_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_IMX662_Basic_960x540.json\"" >> IMX662_MODES.txt
    echo "[mode.3]" >> IMX662_MODES.txt
    echo "xml = \"IMX662_Basic_640x480.xml\"" >> IMX662_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_IMX662_Basic_640x480.json\"" >> IMX662_MODES.txt
}

# Load modules specific to IMX662 - returns module string
load_modules() {
    echo "max96792 max96793 imx662"
}

# Main function to handle commands
main() {
    case "$1" in
        "write_default_modes")
            write_default_modes
            ;;
        "get_1080p_mode")
            echo "0"  # IMX662_Basic_1920x1080.xml
            ;;
        "get_4k_mode")
            echo "0"  # IMX662_Basic_1920x1080.xml (no 4K available)
            ;;
        "get_driver_file")
            echo "imx662.drv"
            ;;
        "get_mode_file")
            echo "IMX662_MODES.txt"
            ;;
        "get_sensor_modules"|"load_modules")
            load_modules
            ;;
        "get_camera_name")
            echo "imx662"
            ;;
        "needs_isp_setup")
            return 0  # IMX662 needs ISP setup
            ;;
        *)
            echo "Usage: $0 {write_default_modes|get_1080p_mode|get_4k_mode|get_driver_file|get_mode_file|get_sensor_modules|get_camera_name|needs_isp_setup|load_modules}"
            exit 1
            ;;
    esac
}

# If script is executed directly, run main function
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
