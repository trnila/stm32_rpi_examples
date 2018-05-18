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
#include <errno.h>

const int ADDR = 0x20;

int i2c_read_byte(int fd, uint8_t address, uint8_t reg, uint8_t *data) {
	if(!data) {
		return -1;
	}

	struct i2c_msg msgs[2]{};
	// send reg address
	msgs[0].addr = address;
	msgs[0].flags = 0;
	msgs[0].buf = &reg;
	msgs[0].len = 1;

	// repeated start with reading byte
	msgs[1].addr = address;
	msgs[1].flags = I2C_M_RD;
	msgs[1].buf = data;
	msgs[1].len = 1;

	struct i2c_rdwr_ioctl_data msg{};
	msg.msgs = msgs;
	msg.nmsgs = 2;

	if(ioctl(fd, I2C_RDWR, &msg) == -1) {
		perror("ioctl");
		return errno;
	}
	return 0;
}

int i2c_write_byte(int fd, uint8_t address, uint8_t reg, uint8_t data) {
	// select slave
	if(ioctl(fd, I2C_SLAVE, address) < 0) {
		perror("ioctl");
		return errno;
	}

	uint8_t frame[] = {reg, data};
	if(write(fd, frame, sizeof(frame)) != sizeof(frame)) {
		perror("error\n");
		return errno;
	}
	return 0;
}

int i2c_write_block(int fd, int address, int reg, uint8_t *data, int size) {
	// select slave
	if(ioctl(fd, I2C_SLAVE, address) < 0) {
		perror("ioctl");
		return errno;
	}

	// first by of frame is register address
	uint8_t frame[size + 1]{};
	frame[0] = reg;
	memcpy(frame + 1, data, size);

	if(write(fd, frame, sizeof(frame)) != sizeof(frame)) {
		perror("error\n");
		return errno;
	}
	return 0;

}

int i2c_read_block(int fd, uint8_t address, uint8_t reg, uint8_t *data, int size) {
	if(!data) {
		return -1;
	}

	struct i2c_msg msgs[2]{};
	// send reg address
	msgs[0].addr = address;
	msgs[0].flags = 0;
	msgs[0].buf = &reg;
	msgs[0].len = 1;

	// repeated start with reading block of data
	msgs[1].addr = address;
	msgs[1].flags = I2C_M_RD;
	msgs[1].buf = data;
	msgs[1].len = size;

	struct i2c_rdwr_ioctl_data msg{};
	msg.msgs = msgs;
	msg.nmsgs = 2;

	if(ioctl(fd, I2C_RDWR, &msg) == -1) {
		perror("ioctl");
		return errno;
	}
	return 0;
}

int main() {
	int fd = open("/dev/i2c-1", O_RDWR);
	if(fd < 0) {
		perror("open");
		exit(1);
	}


	// write single byte
	printf("Write byte 0xAB\n");
	i2c_write_byte(fd, ADDR, 15, 0xAB);

	// receive single byte
	uint8_t rcv = 0;
	printf("Read byte back\n");
	i2c_read_byte(fd, ADDR, 15, &rcv);
	printf("Received %X: %s\n", rcv, rcv == 0xAB ? "OK" : "ERROR");


	// write block of data
	uint8_t data[] = {9, 2, 3, 4, 5};
	printf("Sending block of data\n");
	i2c_write_block(fd, ADDR, 1, data, sizeof(data));

	// read block of data
	uint8_t rcv_block[sizeof(data)]{};
	i2c_read_block(fd, ADDR, 1, rcv_block, sizeof(rcv_block));
	printf("Received block ");
	bool matches = true;
	for(int i = 0; i < sizeof(rcv_block); i++) {
		printf("%d ", rcv_block[i]);
		matches &= data[i] == rcv_block[i];
	}
	printf(matches ? "OK\n" : "ERROR\n");


	// write and read sequentially whole memory
	for(int i = 0; i < 16; i++) {
		i2c_write_byte(fd, ADDR, i, i + 100);
	}
	matches = true;
	for(int i = 0; i < 16; i++) {
		uint8_t recv = 0;
		i2c_read_byte(fd, ADDR, i, &recv);
		printf("%d\n", recv);
		matches &= recv == i + 100;
	}
	printf(matches ? "OK\n" : "ERROR\n");

	// check array overflow
	uint8_t block[8]{};
	i2c_read_block(fd, ADDR, 12, block, sizeof(block));
	for(int i = 0; i < sizeof(block); i++) {
		printf("%d ", block[i]);
	}
	printf("\n");

	return 0;
}
