#! /usr/bin/env bash

OVERLAYS_FILE="/storage/config/overlays"
I2C_OVERLAYS="/storage/config/i2c-overlays"
# get machine from swudpate-config
. "/etc/default/swupdate"
MACHINE=$hardware

function get_slot
{
    for i in `cat /proc/cmdline`; do
        if [[ $i == bootslot=* ]]; then
            CURRENT_SLOT="${i: -1}"
        fi;
    done
    [ "$CURRENT_SLOT" = "1" ] || CURRENT_SLOT="0";
}

i2cdetect_run() {
    get_slot

    # Itterate over the camera devices    
    if [ -f "/boot/bsp${CURRENT_SLOT}/devicetree/cam-overlays" ]; then
        CAM_OVERLAYS="/boot/bsp${CURRENT_SLOT}/devicetree/cam-overlays"
        echo "Loading camera overlays"
        # i2cdetect -y 1 >> /run/initrd_log
        # i2cdetect -y 2 >> /run/initrd_log
        cat $CAM_OVERLAYS | while read -r line || [[ -n "$line" ]]; do
            addr=$(echo $line | awk '{ print $1 }')
            overlay=$(echo $line | awk '{ print $2 }')

            for i in 0 1 2 3 4 5; do
                i2c_name=$(strings "/dev/i2c_names/csi${i}_i2c" 2> /dev/null) || continue
                if i2cget -y "$i2c_name" "$addr" 0x00 > /dev/null 2>&1; then
                    # load the overlay
                    mkdir -p "/sys/kernel/config/device-tree/overlays/${MACHINE}-cam${i}-${overlay%.*}"
                    cat "/boot/bsp${CURRENT_SLOT}/devicetree/${MACHINE}-cam${i}-${overlay}" > "/sys/kernel/config/device-tree/overlays/${MACHINE}-cam${i}-${overlay%.*}/dtbo" || \
                    cat "/boot/bsp${CURRENT_SLOT}/devicetree/*cam${i}-${overlay}" > "/sys/kernel/config/device-tree/overlays/${MACHINE}-cam${i}-${overlay%.*}/dtbo"
                    echo "Found camera $i on i2c bus $i2c_name at address $addr"
                else
                    echo "No camera ${overlay} $i found on i2c bus $i2c_name at address $addr"
                fi
            done
        done
    else
        echo "No camera overlays found"
        return 1
    fi
}

i2cdetect_run
