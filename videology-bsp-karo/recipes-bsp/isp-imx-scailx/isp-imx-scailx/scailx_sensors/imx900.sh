#!/bin/bash

# IMX900 Sensor Configuration Script
# This script contains IMX900-specific functions and configurations

# Write default mode file for IMX900
write_default_modes() {
    echo "Writing IMX900 default modes..."
    echo -n "" > IMX900_MODES.txt
    echo "[mode.0]" >> IMX900_MODES.txt
    echo "xml = \"IMX900_Basic_2048x1536.xml\"" >> IMX900_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_IMX900_Basic_2048x1536.json\"" >> IMX900_MODES.txt
    echo "[mode.1]" >> IMX900_MODES.txt
    echo "xml = \"IMX900_Basic_1920x1080.xml\"" >> IMX900_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_IMX900_Basic_1920x1080.json\"" >> IMX900_MODES.txt
    echo "[mode.2]" >> IMX900_MODES.txt
    echo "xml = \"IMX900_Basic_1024x768.xml\"" >> IMX900_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_IMX900_Basic_1024x768.json\"" >> IMX900_MODES.txt
    echo "[mode.3]" >> IMX900_MODES.txt
    echo "xml = \"IMX900_Basic_2048x154.xml\"" >> IMX900_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_IMX900_Basic_2048x154.json\"" >> IMX900_MODES.txt
    echo "[mode.4]" >> IMX900_MODES.txt
    echo "xml = \"IMX900_Basic_1008x704.xml\"" >> IMX900_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_IMX900_Basic_1008x704.json\"" >> IMX900_MODES.txt
}

# Load modules specific to IMX900 - returns module string
load_modules() {
    echo "max96792 max96793 imx900"
}

# Main function to handle commands
main() {
    case "$1" in
        "write_default_modes")
            write_default_modes
            ;;
        "get_1080p_mode")
            echo "1"  # IMX900_Basic_1920x1080.xml
            ;;
        "get_4k_mode")
            echo "0"  # IMX900_Basic_2048x1536.xml (closest to 4K)
            ;;
        "get_driver_file")
            echo "imx900.drv"
            ;;
        "get_mode_file")
            echo "IMX900_MODES.txt"
            ;;
        "get_sensor_modules"|"load_modules")
            load_modules
            ;;
        "get_camera_name")
            echo "imx900"
            ;;
        "needs_isp_setup")
            return 0  # IMX900 needs ISP setup
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
