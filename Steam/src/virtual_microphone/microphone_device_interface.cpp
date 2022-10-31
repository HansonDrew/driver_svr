#include "microphone_device_interface.h"
#include "microphone_device.h"
#include <mmreg.h>
namespace pxr_base
{
    MicrophoneDeviceInterface* MicrophoneDeviceInterface::Create(
        MicrophoneDeviceCallback* callback)
    {
        return new MicrophoneDevice(callback);
    }

    void MicrophoneDeviceInterface::Destroy(MicrophoneDeviceInterface* val)
    {
        delete val;
        val = nullptr;
    }
}