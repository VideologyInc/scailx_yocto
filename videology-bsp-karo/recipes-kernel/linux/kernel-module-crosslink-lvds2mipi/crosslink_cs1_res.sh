#! /bin/bash
# hacky script which answers an upcall from the crosslink kernel module to chnage the resolution.
# this needs to be changed to either:
# 1: i2c-to-uart inside the crosslink to take care of visca.
# 2: a serdev v4l2 subdev driver which chains with the crosslink one.
# 3: at least a V4L2_EVENT_SOURCE_CHANGE event chnage listener in userspace to replace this script.

port="/dev/ttymxc3"

log=/home/root/lvds2mipi_log.txt

function write_check() {
    for l in {0..20}; do
        res=$(serial-xfer 9600 "$port" "$1")
        echo $res >> $log
        sleep 0.1
	[[ "$res" == *"9041FF"* ]] && break
    done
}

    # Sony
function Sony() {
    if [ ! -z "$1" ]; then

         # 720P25 Sony FCB-EV9520L
        [ "$1" == "0x03" ] && write_check "81010424720101FF" && write_check "81010424740000FF" && write_check "8101041903FF" && echo "Sony 720P25"  >> $log
        # 720P30 Sony FCB-EV9520L
        [ "$1" == "0x02" ] && write_check "8101042472000FFF" && write_check "81010424740000FF" && write_check "8101041903FF" && echo "Sony 720P30"  >> $log
        # 720P50 Sony FCB-EV9520L
        [ "$1" == "0x01" ] && write_check "8101042472000CFF" && write_check "81010424740000FF" && write_check "8101041903FF" && echo "Sony 720P50"  >> $log
        # 720P60 Sony FCB-EV9520L
        [ "$1" == "0x00" ] && write_check "8101042472000AFF" && write_check "81010424740000FF" && write_check "8101041903FF" && echo "Sony 720P60"  >> $log
        # 1080P25 Sony FCB-EV9520L
        [ "$1" == "0x13" ] && write_check "81010424720008FF" && write_check "81010424740000FF" && write_check "8101041903FF" && echo "Sony 1080P25" >> $log
        # 1080P30 Sony FCB-EV9520L
        [ "$1" == "0x12" ] && write_check "81010424720007FF" && write_check "81010424740000FF" && write_check "8101041903FF" && echo "Sony 1080P30" >> $log
        # 1080p50 Sony FCB-EV9520L
        [ "$1" == "0x93" ] && write_check "81010424720104FF" && write_check "81010424740001FF" && write_check "8101041903FF" && echo "Sony 1080P50" >> $log
        # 1080p60 Sony FCB-EV9520L
        [ "$1" == "0x92" ] && write_check "81010424720105FF" && write_check "81010424740001FF" && write_check "8101041903FF" && echo "Sony 1080P60" >> $log
    fi
    for f in {0..50}; do
        res=`serial-xfer 9600 $port 81090400FF`
        if (( "$res" == "905003FF" )); then
            break
        fi
    done
}

function ZoomBlock() {
    if [ ! -z "$1" ]; then
        # 720P25
        [ "$1" == "0x03" ] && write_check "81010424720101FF" && write_check "81010424740000FF"  && echo "ZoomBlock 720P25" >> $log
    	# 720P30
        [ "$1" == "0x02" ] && write_check "8101042472000EFF" && write_check "81010424740000FF"  && echo "ZoomBlock 720P30" >> $log
    	# 720P50
        [ "$1" == "0x01" ] && write_check "8101042472000CFF" && write_check "81010424740000FF"  && echo "ZoomBlock 720P50" >> $log
    	# 720P60
        [ "$1" == "0x00" ] && write_check "81010424720009FF" && write_check "81010424740000FF"  && echo "ZoomBlock 720P60" >> $log
    	# 1080P25
        [ "$1" == "0x13" ] && write_check "81010424720008FF" && write_check "81010424740000FF"  && echo "ZoomBlock 1080P25" >> $log
    	# 1080P30
        [ "$1" == "0x12" ] && write_check "81010424720006FF" && write_check "81010424740000FF"  && echo "ZoomBlock 1080P30" >> $log
        # 1080p50
        [ "$1" == "0x93" ] && write_check "81010424720104FF" && write_check "81010424740001FF"  && echo "ZoomBlock 1080P50" >> $log
        # 1080p60
        [ "$1" == "0x92" ] && write_check "81010424720103FF" && write_check "81010424740001FF"  && echo "ZoomBlock 1080P60" >> $log
    fi
}

# check if Sony or Not
res=$(serial-xfer 9600 "$port" "81090002FF")
# clear log
echo $res > $log
if [[ "$res" == *"0711"* ]]; then
    Sony $1
elif [[ "$res" == *"0466"* ]]; then
    ZoomBlock $1
else
    echo "Unknown camera" >> $log
fi
