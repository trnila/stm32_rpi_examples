#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <time.h>

const int ADDR = 0x20;

// dir
const uint8_t CMD_IODIRA = 0x00;
const uint8_t CMD_IODIRB = 0x01;
// outputs
const uint8_t CMD_OLATA = 0x14;
const uint8_t CMD_OLATB = 0x15;
// inputs
const uint8_t CMD_GPIOA = 0x12;
const uint8_t CMD_GPIOB = 0x13;
// pull ups
const uint8_t CMD_GPPUA = 0x0c;
const uint8_t CMD_GPPUB = 0x0d;

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)
#define panic(msg) perror(msg ":" STRINGIZE(__LINE__)); exit(1)

// i2c fd
int fd;

void i2c_cmd(uint8_t cmd, uint8_t param) {
	uint8_t req[2];
	req[0] = cmd;
	req[1] = param;
	if(write(fd, req, sizeof(req)) != sizeof(req)) {
		panic("write");
	}
	usleep(1000);
}

void gpio_setup(uint8_t bank, uint8_t pins) {
	i2c_cmd(CMD_IODIRA + bank, pins);
}

void gpio_set(uint8_t bank, uint8_t state) {
	i2c_cmd(CMD_GPIOA + bank, state);
}

int main() {
	fd = open("/dev/i2c-1", O_RDWR);
	if(fd < 0) {
		panic("open");
	}

	if(ioctl(fd, I2C_SLAVE, ADDR) < 0) {
		panic("ioctl");
	}

	gpio_setup(0, 0x00);
	gpio_setup(1, 0x00);

	int bank = 0;
	for(;;) {
		for (int i = 0; i < 128; i++) {
			gpio_set(bank, i);
			usleep(10 * 1000);
			printf(".");
			fflush(stdout);
		}
		gpio_set(bank, 0);
		bank = !bank;
		printf("\n");
	}

	return 0;
}
