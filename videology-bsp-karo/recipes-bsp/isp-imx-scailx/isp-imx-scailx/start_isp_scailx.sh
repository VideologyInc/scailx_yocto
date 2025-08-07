#!/bin/sh

SENSORS=$(for f in /proc/device-tree/chosen/overlays/cam*; do basename $f; done)
NUM_SENSORS=$(echo $SENSORS | wc -w)

RUN_SCRIPT=`realpath -s $0`
RUN_SCRIPT_PATH=`dirname $RUN_SCRIPT`
echo "RUN_SCRIPT=$RUN_SCRIPT"
echo "RUN_SCRIPT_PATH=$RUN_SCRIPT_PATH"

cd $RUN_SCRIPT_PATH

RUN_OPTION="DUAL_CAMERA"

write_default_mode_files () {
	# Use sensor-specific scripts to write mode files
	echo "Writing default mode files using sensor scripts..."

	# Check if sensor scripts exist and call them
	for sensor_script in scailx_sensors/*.sh; do
		if [ -x "$sensor_script" ]; then
			echo "Running $sensor_script write_default_modes..."
			"$sensor_script" write_default_modes
		fi
	done
}

# write the sensor config file
write_sensor_cfg_file () {
	local SENSOR_FILE="$1"
	local CAM_NAME="$2"
	local DRV_FILE="$3"
	local MODE_FILE="$4"
	local MODE="$5"

	echo -n "" > $SENSOR_FILE
	echo "name = \"$CAM_NAME\"" >> $SENSOR_FILE
	echo "drv = \"$DRV_FILE\"" >> $SENSOR_FILE
	echo "mode = $MODE" >> $SENSOR_FILE
	cat $MODE_FILE >> $SENSOR_FILE

	if [ ! -f $DRV_FILE ]; then
		echo "File does not exist: $DRV_FILE" >&2
		exit 1
	fi
	if [ ! -f $MODE_FILE ]; then
		echo "File does not exist: $MODE_FILE" >&2
		exit 1
	fi
}

# Function to extract sensor type from sensor name (e.g., "cam0-imx900" -> "imx900")
get_sensor_type() {
    local sensor_name="$1"
    echo "${sensor_name#*-}" | tr '[:upper:]' '[:lower:]'
}

# Function to check if sensor needs ISP setup using sensor scripts
needs_isp_setup() {
    local sensor_type="$1"
    local script_path="scailx_sensors/${sensor_type}.sh"

    if [ -x "$script_path" ]; then
        "$script_path" needs_isp_setup
        return $?
    else
        return 1  # Unknown sensors are skipped by default
    fi
}

write_default_mode_files

echo "Detected sensors: $SENSORS"
echo "Number of sensors: $NUM_SENSORS"

# Check if any sensors were detected
if [ "$NUM_SENSORS" -eq "0" ]; then
    echo "No sensors detected! Please check hardware connections."
    exit 1
fi

echo "Configuring for 1080p mode..."

# Process each sensor first to count ISP sensors
ISP_SENSOR_COUNT=0
for SENSOR in $SENSORS; do
    SENSOR_TYPE=$(get_sensor_type "$SENSOR")
    if needs_isp_setup "$SENSOR_TYPE"; then
        ISP_SENSOR_COUNT=$((ISP_SENSOR_COUNT + 1))
    fi
done

# remove entry.cfg files
rm -f ./*_Entry.cfg
rm -f ./Sensor*_Entry.cfg

# Process each sensor for ISP configuration
for SENSOR in $SENSORS; do
    # Extract sensor number from string (e.g., "cam0-imx900" -> "0")
    SENSOR_NUM=$(echo "$SENSOR" | sed 's/cam\([0-9]*\)-.*/\1/')
    SENSOR_TYPE=$(get_sensor_type "$SENSOR")

    # Check if this sensor needs ISP setup
    if needs_isp_setup "$SENSOR_TYPE"; then
        echo "Processing ISP sensor $SENSOR_NUM: $SENSOR_TYPE"

        # Get configuration for this sensor
        CAM_NAME=$(scailx_sensors/${SENSOR_TYPE}.sh get_camera_name 2>/dev/null || echo "${SENSOR_TYPE}")
        DRV_FILE=$(scailx_sensors/${SENSOR_TYPE}.sh get_driver_file 2>/dev/null || echo "${SENSOR_TYPE}.drv")
        MODE_FILE=$(scailx_sensors/${SENSOR_TYPE}.sh get_mode_file 2>/dev/null || echo "${SENSOR_TYPE}_MODES.txt")
        MODE=$(scailx_sensors/${SENSOR_TYPE}.sh get_1080p_mode 2>/dev/null || echo "0")

        # check if its a single sensor on port 0
        if [ "$SENSOR_NUM" -eq 0 ] && [ "$ISP_SENSOR_COUNT" -eq 1 ]; then
            RUN_OPTION="CAMERA0"
            echo "Using dual-isp mode"
            # MODE=$(scailx_sensors/${SENSOR_TYPE}.sh get_4k_mode 2>/dev/null || echo "0")
            # echo "Using 4K mode for single sensor"
        fi

        # Write configuration file for this sensor
        write_sensor_cfg_file "Sensor${SENSOR_NUM}_Entry.cfg" "$CAM_NAME" "$DRV_FILE" "$MODE_FILE" "$MODE"
    else
        echo "Skipping non-ISP sensor $SENSOR_NUM: $SENSOR_TYPE"
    fi
done

PIDS_TO_KILL=`pgrep -f video_test\|isp_media_server`
if [ ! -z "$PIDS_TO_KILL" ]
then
	echo "Killing preexisting instances of video_test and isp_media_server:"
	echo `ps $PIDS_TO_KILL`
	pkill -f video_test\|isp_media_server
fi

if [ "$ISP_SENSOR_COUNT" -gt "0" ]; then
    modprobe vvcam-video || echo "Failed to load vvcam-video module" >&2
    echo "Starting isp_media_server with configuration file $RUN_OPTION"
    ./isp_media_server $RUN_OPTION || echo "Failed to start isp_media_server" >&2
fi