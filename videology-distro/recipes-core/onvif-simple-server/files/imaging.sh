#! /usr/bin/env bash

DEVICE=/dev/v4l-subdev1

# Don't edit below this line

case $1 in

get_brightness)
    v4l2-ctl -d $DEVICE -C brightness | sed 's/brightness: //'
    ;;

get_color_saturation)
    v4l2-ctl -d $DEVICE -C saturation | sed 's/saturation: //'
    ;;

get_contrast)
    v4l2-ctl -d $DEVICE -C contrast | sed 's/contrast: //'
    ;;

get_sharpness)
    v4l2-ctl -d $DEVICE -C sharpness | sed 's/sharpness: //'
    ;;

get_minmax)
    MINMAX_VALUES=$(echo $(v4l2-ctl -d $DEVICE -l | grep -E 'brightness|contrast|saturation|sharpness' | awk '/min=/ || /max=/ {
        for (i = 1; i <= NF; i++) {
            if ($i ~ /min=/) {
                sub("min=", "", $i);
                min = $i;
            } else if ($i ~ /max=/) {
                sub("max=", "", $i);
                max = $i;
            }
        }
        printf "%s %s ", min, max;
    }'))
    echo "$MINMAX_VALUES"
    ;;

set_brightness)
    v4l2-ctl -d $DEVICE -c brightness=$2
    ;;

set_color_saturation)
    v4l2-ctl -d $DEVICE -c saturation=$2
    ;;

set_contrast)
    v4l2-ctl -d $DEVICE -c contrast=$2
    ;;

set_sharpness)
    v4l2-ctl -d $DEVICE -c sharpness=$2
    ;;

set_default)
    DEFAULT_VALUES=$(echo $(v4l2-ctl -d $DEVICE -l | grep -E 'brightness|contrast|saturation|sharpness' | awk '/default=/ {for (i = 1; i <= NF; i++) {if ($i ~ /default=/) { sub("default=", "", $i); default_val = $i; } } printf "%s ", default_val; } END { print "" }'))
    BRIGHTNESS_DEFAULT=$(echo $DEFAULT_VALUES | cut -f1 -d " ")
    CONTRAST_DEFAULT=$(echo $DEFAULT_VALUES | cut -f2 -d " ")
    SATURATION_DEFAULT=$(echo $DEFAULT_VALUES | cut -f3 -d " ")
    SHARPNESS_DEFAULT=$(echo $DEFAULT_VALUES | cut -f4 -d " ")
    v4l2-ctl -d $DEVICE -c brightness=$BRIGHTNESS_DEFAULT,saturation=$SATURATION_DEFAULT,contrast=$CONTRAST_DEFAULT,sharpness=$SHARPNESS_DEFAULT
    ;;

*)
    exit 0
    ;;
esac

exit 0