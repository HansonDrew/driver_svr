/*----------------------------------------------------------------------------*\
**          Copyright (c) 2017-2018 Qualcomm Technologies, Inc.               **
**                       All Rights Reserved.                                 **
**     Confidential and Proprietary - Qualcomm Technologies, Inc.             **
\*----------------------------------------------------------------------------*/

#pragma once
#include <stdio.h>
#include <mutex>

#include "RVRUtils.h"

/// Define RVR_MODULE_TAG before any header inclusion in cpp file to add
/// module prefix to each logs
#ifdef RVR_MODULE_TAG
/// Buffer related logs
#define RVR_LOG_B(...) Log(RVR_LOG_BUFFER,  RVR_MODULE_TAG, __VA_ARGS__)
/// Verbose logs
#define RVR_LOG_V(...) Log(RVR_LOG_VERBOSE, RVR_MODULE_TAG, __VA_ARGS__)
/// Info logs
#define RVR_LOG_I(...) Log(RVR_LOG_INFO,    RVR_MODULE_TAG, __VA_ARGS__)
/// Warning logs
#define RVR_LOG_W(...) Log(RVR_LOG_WARN,    RVR_MODULE_TAG, __VA_ARGS__)
/// Error logs
#define RVR_LOG_E(...) Log(RVR_LOG_ERROR,   RVR_MODULE_TAG, __VA_ARGS__)
/// Always logs
#define RVR_LOG_A(...) Log(RVR_LOG_ALWAYS,  RVR_MODULE_TAG, __VA_ARGS__)
/// KPI logs
#define RVR_LOG_K(...) Log(RVR_LOG_KPI,     RVR_MODULE_TAG, __VA_ARGS__)
#else
/// Buffer related logs
#define RVR_LOG_B(...) Log(RVR_LOG_BUFFER,  NULL, __VA_ARGS__)
/// Verbose logs
#define RVR_LOG_V(...) Log(RVR_LOG_VERBOSE, NULL, __VA_ARGS__)
/// Info logs
#define RVR_LOG_I(...) Log(RVR_LOG_INFO,    NULL, __VA_ARGS__)
/// Warning logs
#define RVR_LOG_W(...) Log(RVR_LOG_WARN,    NULL, __VA_ARGS__)
/// Error logs
#define RVR_LOG_E(...) Log(RVR_LOG_ERROR,   NULL, __VA_ARGS__)
/// Always logs
#define RVR_LOG_A(...) Log(RVR_LOG_ALWAYS,  NULL, __VA_ARGS__)
/// KPI logs
#define RVR_LOG_K(...) Log(RVR_LOG_KPI,     NULL, __VA_ARGS__)
#endif

namespace RVR
{

/// Log levels
enum RVRLogLevel {
    RVR_LOG_BUFFER,
    RVR_LOG_VERBOSE,
    RVR_LOG_INFO,
    RVR_LOG_KPI,
    RVR_LOG_WARN,
    RVR_LOG_ERROR,
    RVR_LOG_ALWAYS,
    RVR_LOG_NONE
};

/// Log level strings used while logging
char* RVRLogLevelString[];

/// Logging mode
enum RVRLogMode {
    FILE_OUTPUT = 0     ///< Output all the log message to a file.
};

//******************************************************************************
/// Logger class to log debug messages
class RVRLogger
{
public:
    RVRLogger(const char* logFileName, RVRLogLevel logLevel);

    RVRLogger(const char* logFileName);

    virtual ~RVRLogger();

    virtual void Log(RVRLogLevel logLevel, const char* moduleTag,
        const char* formatString, ...);

    virtual void SetLogLevel(int logLevel);

    virtual void LogV(RVRLogLevel logLevel, const char* moduleTag,
        const char* formatString, va_list varArgList);

protected:
    FILE*                   mLogFile;

    std::mutex              mMutex;

    volatile RVRLogLevel    mLogLevel;
};

//******************************************************************************
/// Any modules can use this function to log using global logger instance
extern "C" void RVRUTILS_API RVRUTILS_STDCALL Log(RVRLogLevel logLevel, const char* moduleTag,
    const char* formatString, ...);

extern "C" void RVRUTILS_API RVRUTILS_STDCALL SetLogLevel(int logLevel);
} // Namespace RVR