#pragma once
#include <string>
#include <mmdeviceapi.h>
class AudioDeviceManger
{
public:
	bool StartUp();
	bool ClearUp();
private:
	std::wstring origin_defalut_device_name_ = L"";
	std::wstring origin_defalut_device_id_ = L"";
	bool GetDefaultDevice(IMMDevice** device, int device_type);
	bool GetDefaultDevice(std::wstring& device_id, std::wstring& device_name, int device_type);
	bool SetDefaultDeviceById(std::wstring& device_id);
	bool SetDefaultDeviceByName(std::wstring& device_name, int device_type);
	bool SetDeviceEnable(std::wstring pwszID);
};

