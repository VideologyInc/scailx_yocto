#!/bin/sh

SENSORS=$(for f in /proc/device-tree/chosen/overlays/cam*; do basename $f; done)
NUM_SENSORS=$(echo $SENSORS | wc -w)

RUN_SCRIPT=`realpath -s $0`
RUN_SCRIPT_PATH=`dirname $RUN_SCRIPT`
echo "RUN_SCRIPT=$RUN_SCRIPT"
echo "RUN_SCRIPT_PATH=$RUN_SCRIPT_PATH"

cd $RUN_SCRIPT_PATH

LOAD_MODULES="1" # load modules by default
RUN_OPTION=""
# an array with the modules to load, insertion order
declare -a MODULES=("vvcam-dwe" "vvcam-isp" "vvcam-video")

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

load_module () {
	local MODULE="$1.ko"
	local MODULE_PARAMS="$2"

	# return directly if already loaded.
	MODULENAME=`echo $1 | sed 's/-/_/g'`
	echo $MODULENAME
	if lsmod | grep "$MODULENAME" ; then
		echo "$1 already loaded."
		return 0
	fi

	MODULE_SEARCH=/lib/modules/`uname -r`/
	MODULE_PATH=`find $MODULE_SEARCH -name $MODULE | head -1`
	if [ "$MODULE_PATH" == "" ]; then
		echo "Module $MODULE not found in $MODULE_SEARCH"
		exit 1
	fi
	MODULE_PATH=`realpath $MODULE_PATH`
	insmod $MODULE_PATH $MODULE_PARAMS  || echo "Failed to load module $MODULE_PATH with params $MODULE_PARAMS" >&2
	echo "Loaded $MODULE_PATH $MODULE_PARAMS"
}

load_modules () {
	# remove any previous instances of the modules
	n=${#MODULES_TO_REMOVE[*]}
	for (( i = n-1; i >= 0; i-- ))
	do
		echo "Removing ${MODULES_TO_REMOVE[i]}..."
		rmmod ${MODULES_TO_REMOVE[i]} &>/dev/null
		if lsmod | grep "${MODULES_TO_REMOVE[i]}" ; then
			echo "Removing ${MODULES_TO_REMOVE[i]} failed!"
			# exit 1
		fi
	done

	# and now clean load the modules
	for i in "${MODULES[@]}"
	do
		echo "Loading module $i ..."
		load_module $i
	done
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

# Initialize module collections
MODULES_TO_REMOVE=("basler-camera-driver-vvcam" "os08a20" "ov2775" "imx8-media-dev" "${MODULES[@]}")
ALL_SENSOR_MODULES=()

# Process each sensor first to count ISP sensors
ISP_SENSOR_COUNT=0
for SENSOR in $SENSORS; do
    SENSOR_TYPE=$(get_sensor_type "$SENSOR")
    if needs_isp_setup "$SENSOR_TYPE"; then
        ISP_SENSOR_COUNT=$((ISP_SENSOR_COUNT + 1))
    fi
done

# # Check if any ISP sensors were detected
# if [ "$ISP_SENSOR_COUNT" -eq "0" ]; then
#     echo "No ISP sensors detected! All sensors are non-ISP types."
#     exit 0
# fi

RUN_OPTION="DUAL_CAMERA"

# # Set RUN_OPTION based on number of ISP sensors
# if [ "$ISP_SENSOR_COUNT" -eq "1" ]; then
#     RUN_OPTION="CAMERA0"
#     echo "Configuring single ISP camera setup"
# else
#     RUN_OPTION="DUAL_CAMERA"
#     echo "Configuring multi-camera setup with $ISP_SENSOR_COUNT ISP sensors"
# fi

# remove entry.cfg files
rm -f ./*_Entry.cfg

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

        # Collect modules for this sensor type
        SENSOR_MODULES=$(scailx_sensors/${SENSOR_TYPE}.sh load_modules 2>/dev/null || echo "${SENSOR_TYPE}")
        for MODULE in $SENSOR_MODULES; do
            # Add to modules list if not already present
            if ! [[ " ${ALL_SENSOR_MODULES[@]} " =~ " ${MODULE} " ]]; then
                ALL_SENSOR_MODULES+=("$MODULE")
            fi
        done
    else
        echo "Skipping non-ISP sensor $SENSOR_NUM: $SENSOR_TYPE"
    fi
done

# Update module arrays with collected modules
MODULES_TO_REMOVE=(${ALL_SENSOR_MODULES[@]} "${MODULES_TO_REMOVE[@]}")
MODULES=(${ALL_SENSOR_MODULES[@]} "${MODULES[@]}")

PIDS_TO_KILL=`pgrep -f video_test\|isp_media_server`
if [ ! -z "$PIDS_TO_KILL" ]
then
	echo "Killing preexisting instances of video_test and isp_media_server:"
	echo `ps $PIDS_TO_KILL`
	pkill -f video_test\|isp_media_server
fi

# Need a sure way to wait until all the above processes terminated
sleep 1

if [ "$LOAD_MODULES" == "1" ]; then
	load_modules
fi

echo "Starting isp_media_server with configuration file $RUN_OPTION"
./isp_media_server $RUN_OPTION

# Example GStreamer commands:
# gst-launch-1.0 -v v4l2src device=/dev/video0 ! "video/x-raw,format=YUY2,width=1920,height=1080" ! queue ! imxvideoconvert_g2d ! waylandsink
# gst-launch-1.0 -v v4l2src device=/dev/video0 ! waylandsink
