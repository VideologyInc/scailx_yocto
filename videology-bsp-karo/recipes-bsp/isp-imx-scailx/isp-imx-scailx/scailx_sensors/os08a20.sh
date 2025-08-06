#!/bin/bash

# OS08A20 Sensor Configuration Script
# This script contains OS08A20-specific functions and configurations

# Write default mode file for OS08A20
write_default_modes() {
    echo "Writing OS08A20 default modes..."
    echo -n "" > OS08A20_MODES.txt
    echo "[mode.0]" >> OS08A20_MODES.txt
    echo "xml = \"OS08a20_8M_10_1080p_linear.xml\"" >> OS08A20_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_os08a20_1080P_config.json\"" >> OS08A20_MODES.txt
    echo "[mode.1]" >> OS08A20_MODES.txt
    echo "xml = \"OS08a20_8M_10_1080p_hdr.xml\"" >> OS08A20_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_os08a20_1080P_config.json\"" >> OS08A20_MODES.txt
    echo "[mode.2]" >> OS08A20_MODES.txt
    echo "xml = \"OS08a20_8M_10_4k_linear.xml\"" >> OS08A20_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_os08a20_4K_config.json\"" >> OS08A20_MODES.txt
    echo "[mode.3]" >> OS08A20_MODES.txt
    echo "xml = \"OS08a20_8M_10_4k_hdr.xml\"" >> OS08A20_MODES.txt
    echo "dwe = \"dewarp_config/sensor_dwe_os08a20_4K_config.json\"" >> OS08A20_MODES.txt
}

# Load modules specific to OS08A20 - returns module string
load_modules() {
    echo "os08a20"
}

# Main function to handle commands
main() {
    case "$1" in
        "write_default_modes")
            write_default_modes
            ;;
        "get_1080p_mode")
            echo "0"  # OS08a20_8M_10_1080p_linear.xml
            ;;
        "get_4k_mode")
            echo "2"  # OS08a20_8M_10_4k_linear.xml
            ;;
        "get_driver_file")
            echo "os08a20.drv"
            ;;
        "get_mode_file")
            echo "OS08A20_MODES.txt"
            ;;
        "get_sensor_modules"|"load_modules")
            load_modules
            ;;
        "get_camera_name")
            echo "os08a20"
            ;;
        "needs_isp_setup")
            return 0  # OS08A20 needs ISP setup
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
