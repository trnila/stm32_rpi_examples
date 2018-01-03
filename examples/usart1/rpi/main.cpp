#include <random>
#include <iostream>
#include <chrono>
#include <termios.h>
#include "string.h"
#include "serial.h"
#include "stdio.h"

const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

std::string generate_word(int size) {
	std::default_random_engine eng((std::random_device())());
	std::uniform_int_distribution<int> uni(0, sizeof(chars));

	std::string word;
	word.reserve(size);
	for(int i = 0; i < size; i++) {
		word += chars[uni(eng)];
	}
	return word;
}

const int sizes = 5;

int main() {
	Serial serial("/dev/ttyAMA0", B921600);

	char rcv[sizes];
	auto start = std::chrono::high_resolution_clock::now();
	int sent = 0;
	for(;;) {
		std::string word = generate_word(sizes);
		serial.write(word.c_str(), sizes);

		memset(rcv, 0, sizes);
		serial.read(rcv, sizes);

		if(strncmp(rcv, word.c_str(), sizes) != 0) {
			std::cout << rcv <<  " " << word << "\n";
			return 0;
		}

		sent++;
		double count = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now() - start
		).count();

		std::cout << (int) (sent / count * 1000) << "  \r";
	}
}