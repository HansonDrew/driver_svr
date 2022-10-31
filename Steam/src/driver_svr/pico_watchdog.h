#pragma once
#include <openvr_driver.h>
#include <thread>
void WatchdogThreadFunction();
class PicoWatchdog : public vr::IVRWatchdogProvider
{
public:
	PicoWatchdog()
	{
		m_pWatchdogThread = nullptr;
	}

	virtual vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext);
	virtual void Cleanup();

private:
	std::thread* m_pWatchdogThread;
};