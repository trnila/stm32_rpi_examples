#include <random>
#include <iostream>
#include <chrono>
#include <termios.h>
#include <unistd.h>
#include "string.h"
#include "serial.h"
#include "stdio.h"

int main() {
	Serial serial("/dev/ttyAMA0", B115200);

	//srand(time(NULL));
	srand(0);

	char buffer[128];
	for(;;) {
		int a = rand() % 150;
		int b = rand() % 150;

		snprintf(buffer, sizeof(buffer), "%d + %d\n", a, b);

		printf("%d + %d = ", a, b);

		serial.write(buffer, strlen(buffer));
		serial.readline(buffer, sizeof(buffer));

		int result = 0;
		if(sscanf(buffer, "%d", &result) != 1) {
			printf("Parse error\n");
		} else if(a + b != result) {
			printf("Calculation error\n");
		} else {
			printf("%d\n", result);
		}
	}
}