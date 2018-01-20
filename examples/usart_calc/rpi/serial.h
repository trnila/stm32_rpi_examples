#pragma once
#include "cstring"

class Serial{
public:
	Serial(const char* path, int baud);
	~Serial();

	void write(const char *data, int size);
	void write(const char *data) {
		this->write(data, strlen(data));
	}
	int read(char* buffer, int size);
	int readline(char *buffer, int size);

private:
	int fd;
	char rxBuffer[128];
	int rxPos;

	int setInterfaceAttribs(int speed, int parity);
	void setBlocking(int should_block);
};

