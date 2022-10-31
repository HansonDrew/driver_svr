/*----------------------------------------------------------------------------*\
**          Copyright (c) 2017 Qualcomm Technologies, Inc.                    **
**                       All Rights Reserved.                                 **
**     Confidential and Proprietary - Qualcomm Technologies, Inc.             **
\*----------------------------------------------------------------------------*/

#pragma once

#ifdef RVRUTILS_EXPORTS
#define RVRUTILS_API __declspec(dllexport)
#else
#define RVRUTILS_API __declspec(dllimport)
#endif
#define RVRUTILS_STDCALL __stdcall

#include <Windows.h>
#include "RVRLogger.h"

namespace RVR
{

//******************************************************************************
/// NV12 definition
#ifndef D3DFMT_NV12
#define D3DFMT_NV12 ((D3DFORMAT)MAKEFOURCC('N','V','1','2'))
#endif

//******************************************************************************
/// Enable this macro to cause debug break when any statement using
/// IF_SUCCEEDED macro fails
#define ON_FAILURE_DEBUG_BRK //DebugBreak()

/// Executes given statement only if current status is success and
/// logs failure message if the given statment fails
#define IF_SUCCEEDED(statement) \
    if (SUCCEEDED(status)) \
    { \
        status = statement; \
        if (FAILED(status)) \
        {\
            RVR_LOG_E("%s failed - %d", #statement, status);\
            ON_FAILURE_DEBUG_BRK;\
        }\
    } \

/// Logs failure message if the given statment fails (it uses local  status)
#define LOG_HR_FAILURE(statement) \
    {\
        HRESULT status = statement; \
        if (FAILED(status)) {RVR_LOG_E("\t%s failed - %d", #statement, status);}\
    }\

/// Log error message if the given condition evaluates to false
#define LOG_FAILURE(condition) \
if (!(condition)) \
{ \
    RVR_LOG_W("%s failed", #condition);\
} \

//******************************************************************************
/// Macro to safely release object
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(obj) \
   if (obj != NULL)       \
   {                      \
      obj->Release();     \
      obj = NULL;         \
   }
#endif

/// Macro to safely delete object
#ifndef SAFE_DELETE
#define SAFE_DELETE(obj)  \
   if (obj != NULL)       \
   {                      \
      delete obj;         \
      obj = NULL;         \
   }
#endif

/// Macro to safely close handle
#ifndef SAFE_CLOSE_HANDLE
#define SAFE_CLOSE_HANDLE(obj)  \
   if (obj != NULL)       \
   {                      \
      CloseHandle(obj);         \
      obj = NULL;         \
   }
#endif

//******************************************************************************
#define MAKE_VERSION(Major, Minor) \
    ((Major << 16) | Minor) \

#define MAKE_VERSION_STR(Major, Minor) \
    (#Major"."#Minor) \

//******************************************************************************
/// rvsp wrapper for std::shared_ptr
template <class T>
using rvsp = std::shared_ptr<T>;

/// RVLockScope wrapper for std::lock_guard<std::mutex>
using RVLockScope = std::lock_guard<std::mutex>;

//******************************************************************************
/// GUID format string to print any GUID
#define GUID_FORMAT \
    "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX"

/// GUID_EXPAND expands GUID variables for using it with GUID_FORMAT
#define GUID_EXPAND(guid) \
    guid.Data1, guid.Data2, guid.Data3, \
    guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], \
    guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7] \

//******************************************************************************
/// GetMainWindowHandle to get application main window handle
///
/// \returns    application main window handle
///
//******************************************************************************
HWND GetMainWindowHandle();

//******************************************************************************
/// IsTopWindow
///
/// \returns    true if the given window handle is top window
///
//******************************************************************************
bool IsTopWindow(HWND handle);

//******************************************************************************
/// nowInNs returns current time in terms of nano-second
///
/// \returns    current time in terms of nano-second
///
//******************************************************************************
inline int64_t nowInNs(void) {
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    auto nowInNs = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    return nowInNs;
}

//******************************************************************************
/// nowInUs returns current time in terms of micro-second
///
/// \returns    current time in terms of micro-second
///
//******************************************************************************
inline int64_t nowInUs(void) {
    return nowInNs() / 1000;
}

//******************************************************************************
/// nowInMs returns current time in terms of mili-second
///
/// \returns    current time in terms of mili-second
///
//******************************************************************************
inline int64_t nowInMs(void) {
    return nowInUs() / 1000;
}

//******************************************************************************
/// GetRegistryValue returns given key value from registry
///
/// \returns    given key value from registry
///
//******************************************************************************
RVRUTILS_API std::wstring GetRegistryValue(HKEY key,
    const std::wstring &subKeyName,
    const std::wstring &valueName,
    const std::wstring &defaultValue);

//******************************************************************************
/// GetRegistryValue returns given key value from registry
///
/// \returns    given key value from registry
///
//******************************************************************************
RVRUTILS_API std::string GetRegistryValue(HKEY key,
    const std::string &subKeyName,
    const std::string &valueName,
    const std::string &defaultValue);

//******************************************************************************
/// UInt32ToByteArray fills the given byteArray with the given uint32_t value
/// in network byte order (big-endian)
///
//******************************************************************************
RVRUTILS_API void UInt32ToByteArray(void* byteArray, uint32_t value);

//******************************************************************************
/// FloatToByteArray fills the given byteArray with the given float value
/// in network byte order (big-endian)
///
//******************************************************************************
RVRUTILS_API void FloatToByteArray(void* byteArray, float value);

//******************************************************************************
/// ByteArrayToFloat returns float value from the given byteArray having float
/// value in network byte order (big-endian)
///
/// \returns    float value from the byte array
///
//******************************************************************************
RVRUTILS_API float ByteArrayToFloat(void* byteArray);

//******************************************************************************
/// ByteArrayToInt returns int value from the given byteArray having int
/// value in network byte order (big-endian)
///
/// \returns    int value from the byte array
///
//******************************************************************************
RVRUTILS_API int ByteArrayToInt(void* byteArray);

} // Namespace RVR