#!/bin/bash

# DAA3840 (Basler) Sensor Configuration Script
# This script contains DAA3840-specific functions and configurations

# Write default mode file for DAA3840
write_default_modes() {
    echo "Writing DAA3840 default modes..."
    echo -n "" > DAA3840_MODES.txt
    echo "[mode.0]" >> DAA3840_MODES.txt
    echo "xml = \"DAA3840_30MC_4K-linear.xml\"" >> DAA3840_MODES.txt
    echo "dwe = \"dewarp_config/daA3840_30mc_4K.json\"" >> DAA3840_MODES.txt
    echo "[mode.1]" >> DAA3840_MODES.txt
    echo "xml = \"DAA3840_30MC_1080P-linear.xml\"" >> DAA3840_MODES.txt
    echo "dwe = \"dewarp_config/daA3840_30mc_1080P.json\"" >> DAA3840_MODES.txt
    echo "[mode.2]" >> DAA3840_MODES.txt
    echo "xml = \"DAA3840_30MC_4K-hdr.xml\"" >> DAA3840_MODES.txt
    echo "dwe = \"dewarp_config/daA3840_30mc_4K.json\"" >> DAA3840_MODES.txt
    echo "[mode.3]" >> DAA3840_MODES.txt
    echo "xml = \"DAA3840_30MC_1080P-hdr.xml\"" >> DAA3840_MODES.txt
    echo "dwe = \"dewarp_config/daA3840_30mc_1080P.json\"" >> DAA3840_MODES.txt
}

# Load modules specific to DAA3840 - returns module string
load_modules() {
    echo "basler-camera-driver-vvcam"
}

# Main function to handle commands
main() {
    case "$1" in
        "write_default_modes")
            write_default_modes
            ;;
        "get_1080p_mode")
            echo "1"  # DAA3840_30MC_1080P-linear.xml
            ;;
        "get_4k_mode")
            echo "0"  # DAA3840_30MC_4K-linear.xml
            ;;
        "get_driver_file")
            echo "daA3840_30mc.drv"
            ;;
        "get_mode_file")
            echo "DAA3840_MODES.txt"
            ;;
        "get_sensor_modules"|"load_modules")
            load_modules
            ;;
        "get_camera_name")
            echo "basler-vvcam"
            ;;
        "needs_isp_setup")
            return 0  # DAA3840 needs ISP setup
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
