#!/bin/bash
set -e

if [ -z "$1" ]; then
	echo "Usage: $0 (uart|input)"
	exit 1
fi

trap 'kill $(jobs -p)' EXIT

python3 -m http.server &


if [ "$1" == "uart" ]; then
	stty -F /dev/ttyAMA0 115200
	cat /dev/ttyAMA0  | gossed
elif [ "$1" == "input" ]; then
	make input
	./input | gossed
else 
	echo "Unknown argument: $1"
	exit 1
fi
