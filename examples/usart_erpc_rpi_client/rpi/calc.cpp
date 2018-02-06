#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "setup.h"
#include "messages.h"
#include "Counter.h"
#include <unistd.h>

const int MAXNUM = 100;
const int COUNT = 10;

int32_t sum_local(list_0_t* nums) {
	uint32_t result = 0;

	for(uint32_t i = 0; i < nums->elementsCount; i++) {
		result += nums->elements[i];
	}

	return result;
}

void send_sum() {
	// prepare our list array
	int32_t numbers[COUNT];
	for(int i = 0; i < COUNT; i++) {
		numbers[i] = rand() % MAXNUM;
	}

	// prepare list
	list_0_t list;
	list.elements = numbers;
	list.elementsCount = COUNT;

	// call erpc function and get result
	int32_t result = sum(&list);

	printf("[%s] %d\n", result == sum_local(&list) ? "PASS" : "FAIL", result);
}

void send_addition() {
	int32_t a = rand() % MAXNUM;
	int32_t b = rand() % MAXNUM;
	int32_t result = add(a, b);

	printf("[%s] %d + %d = %d\n", a + b == result ? "PASS" : "FAIL", a, b, result);
}


int main() {
	erpc_setup();
	srand(time(NULL));

	Counter cnt(5);

	//send_sum();
	for(;;) {
		send_addition();
		cnt.tick();
	}

	return 0;
}