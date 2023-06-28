#! /bin/bash
# hacky script which answers an upcall from the crosslink kernel module to chnage the resolution.
# this needs to be changed to either:
# 1: i2c-to-uart inside the crosslink to take care of visca.
# 2: a serdev v4l2 subdev driver which chains with the crosslink one.
# 3: at least a V4L2_EVENT_SOURCE_CHANGE event chnage listener in userspace to replace this script.

port="/dev/ttymxc3"

if [ ! -z "$1" ]; then
    stty 9600 -F "$port"
    # 720P25
    [ "$1" == "0x03" ] && echo -e "\x81\x01\x04\x24\x72\x01\x01\xFF" > "$port" && echo -e "\x81\x01\x04\x24\x74\x00\x00\xFF" > "$port"  # && echo "720P25" > /home/root/res.txt
	# 720P30
    [ "$1" == "0x02" ] && echo -e "\x81\x01\x04\x24\x72\x00\x0E\xFF" > "$port" && echo -e "\x81\x01\x04\x24\x74\x00\x00\xFF" > "$port"  # && echo "720P30" > /home/root/res.txt
	# 720P50
    [ "$1" == "0x01" ] && echo -e "\x81\x01\x04\x24\x72\x00\x0C\xFF" > "$port" && echo -e "\x81\x01\x04\x24\x74\x00\x00\xFF" > "$port"  # && echo "720P50" > /home/root/res.txt
	# 720P60
    [ "$1" == "0x00" ] && echo -e "\x81\x01\x04\x24\x72\x00\x09\xFF" > "$port" && echo -e "\x81\x01\x04\x24\x74\x00\x00\xFF" > "$port"  # && echo "720P60" > /home/root/res.txt
	# 1080P25
    [ "$1" == "0x13" ] && echo -e "\x81\x01\x04\x24\x72\x00\x08\xFF" > "$port" && echo -e "\x81\x01\x04\x24\x74\x00\x00\xFF" > "$port"  # && echo "1080P25" > /home/root/res.txt
	# 1080P30
    [ "$1" == "0x12" ] && echo -e "\x81\x01\x04\x24\x72\x00\x06\xFF" > "$port" && echo -e "\x81\x01\x04\x24\x74\x00\x00\xFF" > "$port"  # && echo "1080P30" > /home/root/res.txt
    # 1080p50
    [ "$1" == "0x93" ] && echo -e "\x81\x01\x04\x24\x72\x01\x04\xFF" > "$port" && echo -e "\x81\x01\x04\x24\x74\x00\x01\xFF" > "$port"  # && echo "1080P50" > /home/root/res.txt
    # 1080p60
    [ "$1" == "0x92" ] && echo -e "\x81\x01\x04\x24\x72\x01\x03\xFF" > "$port" && echo -e "\x81\x01\x04\x24\x74\x00\x01\xFF" > "$port"  # && echo "1080P60" > /home/root/res.txt
fi
