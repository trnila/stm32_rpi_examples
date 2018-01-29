#include <random>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <chrono>
#include <queue>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include "serial.h"
#include "fixedqueue.h"

FixedQueue<std::pair<int, int>> queue(10);

void send(Serial &serial) {
	char buffer[128];

	for(int i = 0; i < 100; i++) {
		int a = rand() % 150;
		int b = rand() % 150;

		queue.push(std::make_pair(a, b));

		snprintf(buffer, sizeof(buffer), "%d + %d\n", a, b);
		serial.write(buffer, strlen(buffer));

		printf("[SENT] %d + %d\n", a, b);
	}
}

int main() {
	Serial serial("/dev/ttyAMA0", B115200);

	//srand(time(NULL));
	srand(0);

	std::thread t(send, std::ref(serial));
	t.detach();

	char buffer[128];
	for(;;) {
		serial.readline(buffer, sizeof(buffer));

		int a, b;
		std::tie(a, b) = queue.get();

		int result = 0;
		if(sscanf(buffer, "%d", &result) != 1) {
			printf("[ERR]  Parse error '%s'\n", buffer);
		} else if(a + b != result) {
			printf("[ERR]  Calculation error %d-%d %d want %d\n", a, b, result, a+b);
		} else {
			printf("[OK]   %d+%d=%d\n", a, b, result);
		}
	}
}