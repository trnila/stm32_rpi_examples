#pragma once

const char *SERIAL_PATH = "/dev/ttyAMA0";
const int BAUD_RATE = 115200;

void erpc_setup(const char* serial_path = SERIAL_PATH, const int baud_rate = BAUD_RATE);