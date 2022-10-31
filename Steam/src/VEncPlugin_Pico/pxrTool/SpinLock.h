#include <atomic>

class SpinLock {

public:
	SpinLock() : flag_(false)
	{}

	void lock()
	{
		bool expect = false;
		while (!flag_.compare_exchange_weak(expect, true))
		{
			expect = false;
		}
	}

	void unlock()
	{
		flag_.store(false);
	}

private:
	std::atomic<bool> flag_;
};