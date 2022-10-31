#include <thread>
#include "pico_watchdog.h"
#include "driverlog.h"
extern bool g_b_exiting;
void WatchdogThreadFunction()
{
	while (!g_b_exiting)
	{
 

		//if (false)
		//{
		//	// Y key was pressed. 
		//	vr::VRWatchdogHost()->WatchdogWakeUp(TrackedDeviceClass_HMD);
		//}
		std::this_thread::sleep_for(std::chrono::microseconds(500));
 
	}
}


bool g_b_exiting = false;

vr::EVRInitError PicoWatchdog::Init(vr::IVRDriverContext* pDriverContext)
{
	VR_INIT_WATCHDOG_DRIVER_CONTEXT(pDriverContext);
	InitDriverLog(vr::VRDriverLog());

	// Watchdog mode on Windows starts a thread that listens for the 'Y' key on the keyboard to 
	// be pressed. A real driver should wait for a system button event or something else from the 
	// the hardware that signals that the VR system should start up.
	g_b_exiting = false;
	m_pWatchdogThread = new std::thread(WatchdogThreadFunction);
	if (!m_pWatchdogThread)
	{
		DriverLog("Unable to create watchdog thread\n");
		return vr::VRInitError_Driver_Failed;
	}

	return vr::VRInitError_None;
}


void PicoWatchdog::Cleanup()
{
	g_b_exiting = true;
	if (m_pWatchdogThread)
	{
		m_pWatchdogThread->join();
		delete m_pWatchdogThread;
		m_pWatchdogThread = nullptr;
	}

	CleanupDriverLog();
}

