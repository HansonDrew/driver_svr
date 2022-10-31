#pragma once
//#if defined(WIN32) || defined(_WINDOWS)
//#ifdef PXR_EXPORTS
//#define PXR_API __declspec(dllexport)
//#else
//#define PXR_API __declspec(dllimport)
//#endif// PXR_EXPORTS
//#else
//#define PXR_API
//#endif// WIN32
#define PXR_API
#include <string>
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
	      	      { (punk)->Release(); (punk) = NULL; }
namespace pxr_base
{
    class PXR_API MicrophoneDeviceCallback
    {
    public:
        enum class LogLevel
        {
            kLogLevelDebug = 0,
            kLogLevelInfo = 0,
            kLogLevelError = 0,
        };

        virtual void NotifyLogCallback(LogLevel level, const char* msg) = 0;
    protected:
        virtual ~MicrophoneDeviceCallback() = default;
    };

    class PXR_API  MicrophoneDeviceInterface
    {
    public:
        static MicrophoneDeviceInterface* Create(MicrophoneDeviceCallback* callback);
        static void Destroy(MicrophoneDeviceInterface* val);

        virtual bool Start(bool wireless_mode=true) = 0;
        virtual void Stop() = 0;
        virtual bool Write(char* buff, int len) = 0;
        virtual bool SetVolum(int level) = 0;
        ~MicrophoneDeviceInterface() = default;
    };
}