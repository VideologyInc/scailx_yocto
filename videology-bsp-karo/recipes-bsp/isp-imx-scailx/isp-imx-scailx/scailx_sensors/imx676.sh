#!/bin/bash

# IMX676 Sensor Configuration Script
# This script contains IMX676-specific functions and configurations

# Write default mode file for IMX676
write_default_modes() {
    echo "Writing IMX676 default modes..."
    echo -n "" > IMX676_MODES.txt
    echo "[mode.0]" >> IMX676_MODES.txt
    echo "xml = \"IMX676_Basic_3536x3072.xml\"" >> IMX676_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_IMX676_Basic_3536x3072.json\"" >> IMX676_MODES.txt
    echo "[mode.1]" >> IMX676_MODES.txt
    echo "xml = \"IMX676_Basic_3536x2140.xml\"" >> IMX676_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_IMX676_Basic_3536x2140.json\"" >> IMX676_MODES.txt
    echo "[mode.2]" >> IMX676_MODES.txt
    echo "xml = \"IMX676_Basic_1776x1778.xml\"" >> IMX676_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_IMX676_Basic_1776x1778.json\"" >> IMX676_MODES.txt
    echo "[mode.3]" >> IMX676_MODES.txt
    echo "xml = \"IMX676_Basic_1760x1070.xml\"" >> IMX676_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_IMX676_Basic_1760x1070.json\"" >> IMX676_MODES.txt
}

# Load modules specific to IMX676 - returns module string
load_modules() {
    echo "max96792 max96793 imx676"
}

# Main function to handle commands
main() {
    case "$1" in
        "write_default_modes")
            write_default_modes
            ;;
        "get_1080p_mode")
            echo "3"  # IMX676_Basic_1760x1070.xml (closest to 1080p)
            ;;
        "get_4k_mode")
            echo "0"  # IMX676_Basic_3536x3072.xml (highest resolution)
            ;;
        "get_driver_file")
            echo "imx676.drv"
            ;;
        "get_mode_file")
            echo "IMX676_MODES.txt"
            ;;
        "get_sensor_modules"|"load_modules")
            load_modules
            ;;
        "get_camera_name")
            echo "imx676"
            ;;
        "needs_isp_setup")
            return 0  # IMX676 needs ISP setup
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
