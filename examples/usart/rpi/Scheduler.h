#pragma once

#include <functional>
#include <chrono>
#include <iostream>
#include <thread>

void schedule(std::chrono::milliseconds delay, std::function<void()> cb) {
	std::thread t([] (std::chrono::milliseconds delay, std::function<void()> cb) {
		for(;;) {
			std::this_thread::sleep_for(delay);
			cb();
		}
	}, delay, cb);
	t.detach();
}

