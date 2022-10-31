#include "Semaphore.h"

pxrSemaphore::pxrSemaphore(int accessCount)
	: accessCount(accessCount)
{
}

void pxrSemaphore::Signal()
{
	std::unique_lock<std::mutex> lock(mutex);
	++accessCount;
	cv.notify_one();
}

void pxrSemaphore::Wait()
{
	std::unique_lock<std::mutex> lock(mutex);
	cv.wait(lock, [=] { return accessCount > 0; });
	--accessCount;
}
