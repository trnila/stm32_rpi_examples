#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <time.h>

const int ADDR = 10;

const uint8_t CMD_READ = 0x10;
const uint8_t CMD_SET = 0x11;
const uint8_t CMD_ADD = 0x12;
const uint8_t CMD_SUB = 0x13;
const uint8_t CMD_AND = 0x14;
const uint8_t CMD_OR = 0x15;


uint32_t calc_read(int fd) {
	uint8_t cmd = CMD_READ;
	uint8_t rxbuffer[4] = {0};

	struct i2c_msg msgs[2];
	msgs[0].addr = ADDR;
	msgs[0].flags = 0;
	msgs[0].buf = &cmd;
	msgs[0].len = 1;

	msgs[1].addr = ADDR;
	msgs[1].flags = I2C_M_RD;
	msgs[1].buf = rxbuffer;
	msgs[1].len = sizeof(rxbuffer);

	struct i2c_rdwr_ioctl_data msg;
	msg.msgs = msgs;
	msg.nmsgs = 2;

	if(ioctl(fd, I2C_RDWR, &msg) == -1) {
		perror("ioctl");
	}

	usleep(1000);
	return (rxbuffer[3] << 24) | (rxbuffer[2] << 16) | (rxbuffer[1] << 8) | rxbuffer[0];
}

void calc_cmd(int fd, uint8_t cmd, int num) {
	uint8_t txbuffer[5];

	txbuffer[0] = cmd;
	txbuffer[1] = num & 0xFF;
	txbuffer[2] = (num >> 8) & 0xFF;
	txbuffer[3] = (num >> 16) & 0xFF;
	txbuffer[4] = (num >> 24) & 0xFF;

	if(write(fd, txbuffer, sizeof(txbuffer)) != sizeof(txbuffer)) {
		perror("error\n");
	}

	usleep(1000);
}

int main() {
	int fd = open("/dev/i2c-1", O_RDWR);
	if(fd < 0) {
		perror("open");
		exit(1);
	}

	if(ioctl(fd, I2C_SLAVE, ADDR) < 0) {
		perror("ioctl");
		exit(1);
	}

	uint32_t res;

	res = calc_read(fd);
	printf("%lu\n", res);

	calc_cmd(fd, CMD_ADD, 1);

	res = calc_read(fd);
	printf("%lu\n", res);

	return 0;
}
