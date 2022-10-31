#pragma once

namespace parfait {

#if defined(Win32) || defined(_WIN32)
#ifdef BUILD_PARFAIT_DLL
#define PARFAIT_API __declspec(dllexport)
#elif defined(BUILD_PARFAIT_LIB)
#define PARFAIT_API
#else
#define PARFAIT_API __declspec(dllimport)
#endif
#else
#define PARFAIT_API __attribute__((__visibility__("default")))
#endif

#ifdef __GNUC__
#define GCC_VERSION_AT_LEAST(x,y) (__GNUC__ > (x) || __GNUC__ == (x) && __GNUC_MINOR__ >= (y))
#else
#define GCC_VERSION_AT_LEAST(x,y) 0
#endif

#if GCC_VERSION_AT_LEAST(3,1)
#define attribute_deprecated __attribute__((deprecated))
#elif defined(_MSC_VER)
#define attribute_deprecated __declspec(deprecated)
#else
#define attribute_deprecated
#endif

#define LOG_VERBOSE_STR "verbose"
#define LOG_DEBUG_STR "debug"
#define LOG_INFO_STR "info"
#define LOG_WARN_STR "warn"
#define LOG_ERROR_STR "error"

enum RECORD_INTERVAL {
    INTERVAL_1 = 1 * 1000,
    INTERVAL_10 = 10 * 1000,
    INTERVAL_15 = 15 * 1000,
    INTERVAL_30 = 30 * 1000
};

enum LOG_RECORD_LEVEL {
    LOG_VERBOSE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
};

enum ALOG_LEVEL {
    ALOG_VERBOSE,
    ALOG_DEBUG,
    ALOG_INFO,
    ALOG_WARN,
    ALOG_ERROR,
};

} //namespace parfait
