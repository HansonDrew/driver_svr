//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#include <openvr_driver.h>

#if defined( _WINDOWS )
#include <windows.h>
#endif

#if defined(_WIN32)
#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )
#define HMD_DLL_IMPORT extern "C" __declspec( dllimport )
#elif defined(__GNUC__) || defined(COMPILER_GCC) || defined(__APPLE__)
#define HMD_DLL_EXPORT extern "C" __attribute__((visibility("default")))
#define HMD_DLL_IMPORT extern "C" 
#else
#error "Unsupported Platform."
#endif

#include "pico_watchdog.h"

class PicoVRDriver;
extern PicoVRDriver g_svrDriver;

PicoWatchdog g_watchdogDriverNull;


//----------------------------------------------------------------------------------
HMD_DLL_EXPORT void* HmdDriverFactory(const char *pInterfaceName, int *pReturnCode)
//----------------------------------------------------------------------------------
{
	if (0 == strcmp(vr::IServerTrackedDeviceProvider_Version, pInterfaceName))
	{
		return &g_svrDriver;
	}
	if (0 == strcmp(vr::IVRWatchdogProvider_Version, pInterfaceName))
	{
		return &g_watchdogDriverNull;
	}

	if (pReturnCode)
		*pReturnCode = vr::VRInitError_Init_InterfaceNotFound;

	return NULL;
}
