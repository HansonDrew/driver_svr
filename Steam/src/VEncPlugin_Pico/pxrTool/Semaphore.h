#pragma once
#include <mutex>

class pxrSemaphore
{
public:
	explicit pxrSemaphore(int accessCount = 0);
	void Signal();
	void Wait();

private:
	std::mutex mutex;
	std::condition_variable cv;
	int accessCount;
};

