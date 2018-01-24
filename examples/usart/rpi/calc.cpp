#include <stdio.h>
#include <stdlib.h>
#include "setup.h"
#include "messages.h"

int main() {
	srand(0);

	erpc_setup();


	for(;;) {
		int a = rand() % 100;
		int b = rand() % 100;
		int result = add(a, b);

		printf("[%s] %d + %d = %d\n", a + b == result ? "PASS" : "FAIL", a, b, result);
	}

	return 0;
}