#!/bin/bash

trap 'kill $(jobs -p)' EXIT

python3 -m http.server &

stty -F /dev/ttyAMA0 115200
cat /dev/ttyAMA0  | gossed
