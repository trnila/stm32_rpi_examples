#!/bin/bash

reset_pin=4
boot1_pin=5

echo $reset_pin > /sys/class/gpio/export
echo $boot1_pin > /sys/class/gpio/export

pin_dir() {
	echo "$2" > /sys/class/gpio/gpio$1/direction
}

pin_set() {
	echo "$2" > /sys/class/gpio/gpio$1/value
}

reset() {
	pin_set $reset_pin 0
	pin_set $reset_pin 1
}

pin_dir $reset_pin out
pin_dir $boot1_pin out

pin_set $boot1_pin 1
reset
python ~/stm32loader.py -p /dev/ttyAMA0 -e -w "$1"
pin_set $boot1_pin 0
reset
pin_dir $reset_pin in
pin_dir $boot1_pin in
