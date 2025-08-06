#!/bin/bash

# IMX678 Sensor Configuration Script
# This script contains IMX678-specific functions and configurations

# Write default mode file for IMX678
write_default_modes() {
    echo "Writing IMX678 default modes..."
    echo -n "" > IMX678_MODES.txt
    echo "[mode.0]" >> IMX678_MODES.txt
    echo "xml = \"IMX678_Basic_3840x2160.xml\"" >> IMX678_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_IMX678_Basic_3840x2160.json\"" >> IMX678_MODES.txt
    echo "[mode.1]" >> IMX678_MODES.txt
    echo "xml = \"IMX678_Basic_1920x1080.xml\"" >> IMX678_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_IMX678_Basic_1920x1080.json\"" >> IMX678_MODES.txt
}

# Load modules specific to IMX678 - returns module string
load_modules() {
    echo "max96792 max96793 imx678"
}

# Main function to handle commands
main() {
    case "$1" in
        "write_default_modes")
            write_default_modes
            ;;
        "get_1080p_mode")
            echo "1"  # IMX678_Basic_1920x1080.xml
            ;;
        "get_4k_mode")
            echo "0"  # IMX678_Basic_3840x2160.xml (true 4K)
            ;;
        "get_driver_file")
            echo "imx678.drv"
            ;;
        "get_mode_file")
            echo "IMX678_MODES.txt"
            ;;
        "get_sensor_modules"|"load_modules")
            load_modules
            ;;
        "get_camera_name")
            echo "imx678"
            ;;
        "needs_isp_setup")
            return 0  # IMX678 needs ISP setup
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
