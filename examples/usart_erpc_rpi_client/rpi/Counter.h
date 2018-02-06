#include <chrono>
#include <algorithm>
#include <vector>

class Counter {
public:
	Counter(int lastValues): pos(0), seconds(1) {
		increments.resize(lastValues);
		reset();
	}

	void inc() {
		cnt++;
	}

	double avg() {
		if(seconds <= 1) {
			return cnt;
		}

		int total = 0;
		for(int cnt: increments) {
			total += cnt;
		}

		return total / std::min((int) increments.size(), seconds - 1);
	}

	void tick() {
		inc();

		if(secondsElapsed() > 1) {
			increments[pos] = cnt;
			pos = (pos + 1) % increments.capacity();
			reset();
			seconds++;
		}

		printf("%d\n", (int) avg());
	}

private:
	std::chrono::high_resolution_clock::time_point start;
	int pos;
	int cnt;
	int seconds;

	std::vector<int> increments;

	int secondsElapsed() {
		return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count();
	}

	void reset() {
		start = std::chrono::high_resolution_clock::now();
		cnt = 0;
	}
};