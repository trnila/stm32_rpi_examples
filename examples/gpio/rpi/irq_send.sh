#!/bin/sh
set -e
set -x
PIN=13

if [ ! -d "/sys/class/gpio/gpio${PIN}" ]; then
	echo $PIN > /sys/class/gpio/export
fi

echo out > /sys/class/gpio/gpio${PIN}/direction

while true; do
	echo 0 > /sys/class/gpio/gpio${PIN}/value
	sleep 1
	echo 1 > /sys/class/gpio/gpio${PIN}/value
	sleep 1
done

