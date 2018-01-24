#include <stdio.h>
#include <unistd.h>
#include <array>
#include <vector>
#include <mutex>
#include "messages.h"
#include "Scheduler.h"
#include "setup.h"

using namespace std::literals::chrono_literals;

std::mutex mutex;

class ChangeSpeed {
public:
	ChangeSpeed(std::vector<int> speeds) {
		this->lastSpeed = 0;
		this->speeds = speeds;
	}

	void operator()() {
		std::lock_guard<std::mutex> guard(mutex);

		std::cout << "changing speed: " << lastSpeed << " " << speeds[lastSpeed] << "\n";
		led_strip(speeds[lastSpeed], Right);
		lastSpeed = (lastSpeed + 1) % speeds.size();
	}

private:
	std::vector<int> speeds;
	int lastSpeed;
};

class Pauser {
public:
	Pauser(): pause(false) {}

	void operator()() {
		std::lock_guard<std::mutex> guard(mutex);
		led_ctrl(pause ? Ctrl_pause : Ctrl_start);
		pause = !pause;
	}

private:
	bool pause;
};

int main() {
	erpc_setup();

	std::vector<int> speeds = {100, 500, 250, 10000};

	schedule(1s, ChangeSpeed(speeds));
//	schedule(2000ms, Pauser());

	bool state = 1;
	for(;;) {
		for(int led = 0; led < 8; led++) {
			{
				const int bank = 1;
				const int state = 1 << led;
				std::lock_guard<std::mutex> guard(mutex);
				printf("set_led(%d, 0x%X) = 0x%X\n", bank, state, set_led(bank, state));
			}

			usleep(100 * 1000);
		}
		state = !state;
	}
}
