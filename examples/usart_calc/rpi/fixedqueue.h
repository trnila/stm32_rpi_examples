#pragma once

#include <condition_variable>
#include <mutex>

template<typename T>
class FixedQueue {
public:
	FixedQueue(int maxItems): maxItems(maxItems) {}

	auto get() {
		std::unique_lock<std::mutex> lck(mutex);

		condition.wait(lck, [&] {return !queue.empty();});

		auto ret = queue.front();
		queue.pop();
		condition.notify_one();
		return ret;
	}

	void push(T item) {
		{
			std::unique_lock <std::mutex> lck(mutex);
			condition.wait(lck, [&] { return queue.size() < maxItems; });
			queue.push(item);
		}
		condition.notify_one();
	}

private:
	int maxItems;
	std::queue<T> queue;
	std::mutex mutex;
	std::condition_variable condition;
};
