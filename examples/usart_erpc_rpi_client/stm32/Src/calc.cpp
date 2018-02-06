#include "messages.h"

int32_t add(int32_t a, int32_t b) {
	return a + b;
}

int32_t sum(const list_0_t * nums) {
	uint32_t result = 0;

	for(uint32_t i = 0; i < nums->elementsCount; i++) {
		result += nums->elements[i];
	}

	return result;
}