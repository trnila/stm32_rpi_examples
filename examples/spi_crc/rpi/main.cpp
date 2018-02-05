/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <time.h>
#include "CRC.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define BUFSIZE 64

static void pabort(const char *s) {
	perror(s);
	abort();
}

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 10000;
static uint16_t delay;
static const char *ttyDevice = "/dev/ttyAMA0";
static int ttySpeed = B115200;

int map_bauds(int bauds) {
	switch(bauds) {
		case 50: return B50;
		case 75: return B75;
		case 110: return B110;
		case 134: return B134;
		case 150: return B150;
		case 200: return B200;
		case 300: return B300;
		case 600: return B600;
		case 1200: return B1200;
		case 1800: return B1800;
		case 2400: return B2400;
		case 4800: return B4800;
		case 9600: return B9600;
		case 19200: return B19200;
		case 38400: return B38400;
		case 57600: return B57600;
		case 115200: return B115200;
		case 230400: return B230400;
		case 460800: return B460800;
		case 500000: return B500000;
		case 576000: return B576000;
		case 921600: return B921600;
		case 1000000: return B1000000;
		case 1152000: return B1152000;
		case 1500000: return B1500000;
		case 2000000: return B2000000;
		case 2500000: return B2500000;
		case 3000000: return B3000000;
		case 3500000: return B3500000;
		case 4000000: return B4000000;
	}

	fprintf(stderr, "Unknown baud rate\n");
	exit(1);
}

static void transfer(int fd, uint8_t *tx, uint8_t *rx, int length) {
	struct spi_ioc_transfer tr;
	memset(&tr, 0, sizeof(tr));
	tr.tx_buf = (unsigned long) tx;
	tr.rx_buf = (unsigned long) rx;
	tr.len = length;
	tr.delay_usecs = delay;
	tr.speed_hz = speed;
	tr.bits_per_word = bits;

	int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

	/*for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", rx[ret]);
	}
	puts("");*/
}

static void print_usage(const char *prog) {
	printf("Usage: %s [-DsbdlHOLC3]\n", prog);
	puts("  -D --device    device to use (default /dev/spidev1.1)\n"
	     "  -s --speed     max speed (Hz)\n"
	     "  -d --delay     delay (usec)\n"
	     "  -b --bpw       bits per word \n"
	     "  -l --loop      loopback\n"
	     "  -H --cpha      clock phase\n"
	     "  -O --cpol      clock polarity\n"
	     "  -L --lsb       least significant bit first\n"
	     "  -C --cs-high   chip select active high\n"
	     "  -3 --3wire     SI/SO signals shared\n"
	     "  -t --ttydevice path for serial console (default /dev/ttyAMA0)\n"
	     "  -T --ttyspeed  baud rate for serial console (default 115200)\n"
	);
	exit(1);
}

static void parse_opts(int argc, char *argv[]) {
	while (1) {
		static const struct option lopts[] = {
				{"device",    1, 0, 'D'},
				{"speed",     1, 0, 's'},
				{"delay",     1, 0, 'd'},
				{"bpw",       1, 0, 'b'},
				{"loop",      0, 0, 'l'},
				{"cpha",      0, 0, 'H'},
				{"cpol",      0, 0, 'O'},
				{"lsb",       0, 0, 'L'},
				{"cs-high",   0, 0, 'C'},
				{"3wire",     0, 0, '3'},
				{"no-cs",     0, 0, 'N'},
				{"ready",     0, 0, 'R'},
				{"ttydevice", 1, 0, 't'},
				{"ttyspeed", 1, 0, 'T'},
				{NULL,      0, 0, 0},
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:t:T:ltHOLC3NR", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
			case 'D':
				device = optarg;
				break;
			case 's':
				speed = atoi(optarg);
				break;
			case 'd':
				delay = atoi(optarg);
				break;
			case 'b':
				bits = atoi(optarg);
				break;
			case 'l':
				mode |= SPI_LOOP;
				break;
			case 'H':
				mode |= SPI_CPHA;
				break;
			case 'O':
				mode |= SPI_CPOL;
				break;
			case 'L':
				mode |= SPI_LSB_FIRST;
				break;
			case 'C':
				mode |= SPI_CS_HIGH;
				break;
			case '3':
				mode |= SPI_3WIRE;
				break;
			case 'N':
				mode |= SPI_NO_CS;
				break;
			case 'R':
				mode |= SPI_READY;
				break;
			case 't':
				ttyDevice = optarg;
				break;
			case 'T':
				ttySpeed = map_bauds(atoi(optarg));
				break;
			default:
				print_usage(argv[0]);
				break;
		}
	}
}

int serial_open(const char* path, int speed) {
	int serialFd = open(path, O_RDWR | O_NOCTTY | O_SYNC);
	if (serialFd < 0) {
		pabort("serial open");
	}

	struct termios tty;
	memset(&tty, 0, sizeof(tty));
	if (tcgetattr(serialFd, &tty) != 0) {
		pabort("tcgetattr");
	}

	cfsetospeed(&tty, B115200);
	cfsetispeed(&tty, B115200);
	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	tty.c_iflag &= ~IGNBRK;
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	tty.c_cflag |= (CLOCAL | CREAD);
	tty.c_cflag &= ~(PARENB | PARODD);
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	// blocking read - wait for 1 byte indefinitely
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 0;

	if (tcsetattr(serialFd, TCSANOW, &tty) != 0) {
		pabort("tcsetattr");
	}

	return serialFd;
}

void wait_for_completion(int fd) {
	char c;
	read(fd, &c, 1);
	printf("%c %d\n", c, c);
}

int checksum(int spiFd, int serialFd, uint8_t *data) {
	uint8_t rx[BUFSIZE];

	uint32_t crc = CRC::Calculate(data, BUFSIZE, CRC::CRC_32_MPEG2());

	transfer(spiFd, data, rx, BUFSIZE);
	wait_for_completion(serialFd);
	transfer(spiFd, data, rx, BUFSIZE);

	uint32_t receivedCrc = ((uint32_t *) rx)[0];

	int match = crc == receivedCrc;
	printf("CRC %s: computed=%x received=%x\n", match ? "MATCH" : "INVALID", crc, receivedCrc);

	return match;
}

int main(int argc, char *argv[]) {
	int ret = 0;
	int fd;

	parse_opts(argc, argv);

	int serialFd = serial_open(ttyDevice, ttySpeed);

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed / 1000);

	srand(time(NULL));

	uint8_t buffer[BUFSIZE];
	for(int i = 0; i < 10; i++) {
		for(int i = 0; i < BUFSIZE; i++) {
			buffer[i] = rand() % 255;
		}

		if(!checksum(fd, serialFd, buffer)) {
			ret = 1;
		}
	}

	close(fd);
	close(serialFd);
	return ret;
}
