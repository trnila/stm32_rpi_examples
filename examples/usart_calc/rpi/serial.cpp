#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <iostream>
#include <termios.h>
#include <assert.h>
#include "serial.h"

Serial::Serial(const char* path, int baud) {
	rxPos = 0;

	fd = open(path, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0) {
		throw std::runtime_error("could not open serial");
	}

	setInterfaceAttribs(baud, 0);
	//setBlocking(0);
}

Serial::~Serial() {
	close(fd);
}

int Serial::setInterfaceAttribs(int speed, int parity) {
	struct termios tty;
	memset(&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0) {
		throw std::runtime_error("tcgetattr failed");
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	tty.c_iflag &= ~IGNBRK;
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_cc[VMIN]  = 0;
	tty.c_cc[VTIME] = 5;

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
	// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr (fd, TCSANOW, &tty) != 0) {
		throw std::runtime_error("tcsetattr failed");
	}
	return 0;
}

void Serial::setBlocking(int should_block) {
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0) {
		throw std::runtime_error("tcgetattr failed");
	}

	tty.c_cc[VMIN]  = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 50;

	if (tcsetattr (fd, TCSANOW, &tty) != 0) {
		throw std::runtime_error("tcsetattr failed");
	}
}

void Serial::write(const char* data, int size) {
	::write(fd, data, size);
}

int Serial::read(char* buffer, int size) {
	return ::read(fd, buffer, size);
}

int Serial::readline(char *buffer, int size) {
	char *pos;
	do {
		pos = (char*) memchr(rxBuffer, '\n', rxPos);
		if (pos) {
			int length = pos - rxBuffer;
			if (length >= size) {
				return 0;
			}

			// copy to receiver buffer
			memcpy(buffer, rxBuffer, length);
			buffer[length] = 0;

			// move the rest of received data
			memmove(rxBuffer, rxBuffer + length, sizeof(rxBuffer) - length - 1);

			rxPos -= length + 1;
			assert(rxPos >= 0);

			return length;
		}

		rxPos += ::read(fd, rxBuffer + rxPos, sizeof(rxBuffer) - rxPos);

		if(rxPos >= sizeof(rxBuffer)) {
			return 0;
		}
	} while(pos == NULL);
}