#include <stdint.h>
#include <string.h>
#include "stm32f1xx.h"

#define SYS_OPEN   0x01
#define SYS_CLOSE  0x02
#define SYS_WRITEC 0x03
#define SYS_WRITE  0x05

unsigned int semihosting(int sys, void *args) {
	unsigned int result;
	asm (
		"mov r0, %[sys]\n"
		"mov r1, %[args]\n"
		"bkpt #0xAB\n"
        "mov %[result], r0\n"
		: [result] "=r"(result)
		: [args] "r"(args), [sys] "r"(sys)
		: "r0", "r1"
	);
	return result;
}

void put_char(char c) {
	semihosting(SYS_WRITEC, &c);
}

int _open(char* file, int flags, int mode) {
	int args[] = {
			file,
			7,
			strlen(file)
	};
	return semihosting(SYS_OPEN, args);
}

int _close(int fildes) {
	return semihosting(SYS_CLOSE, &fildes);
}

int _write(int file, char *data, int len) {
	int args[] = {file, data, len};
	return semihosting(SYS_WRITE, args);
}


void demo() {
	int i = 0;
	FILE* ff = fopen("/tmp/output", "w+");

	for(;;) {
		fprintf(ff, "hello file %d\n", i);
		fflush(ff);

		printf("Hello world %d\n", i);

		i++;
		HAL_Delay(1000);
	}
}