#!/bin/sh
set -e
rtc="$1"

if [ -h /dev/rtc ];then
    target="$(readlink /dev/rtc)"
    if [ "$ACTION" = "remove" ];then
	if [ "$target" = "$rtc" ];then
	    rm -f /dev/rtc
	    shopt -s nullglob
	    for rtc in /sys/class/rtc/rtc*;do
		ln -s "$rtc" /dev/rtc
		break
	    done
	fi
    else
	if [ "$target" != "$rtc" ];then
	    if ! grep -q 'i2c:' "/sys/class/rtc/${rtc}/device/modalias" || [ ! -e /dev/rtc ];then
		rm -f /dev/rtc
		ln -s "$rtc" /dev/rtc
	    fi
	fi
    fi
fi
