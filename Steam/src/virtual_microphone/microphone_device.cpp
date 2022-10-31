
#include <locale.h>
#include <stdio.h>
#include <codecvt>
#include <sstream>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <audioclient.h>
#include <fstream>
#include <vector>
#include <Mmsystem.h>
#include <devguid.h>
#include <shlobj.h>

#include <endpointvolume.h>
#include "microphone_device.h"
#define PXR_TRACE_LOG(M) {std::stringstream ss; ss << M; \
     TraceLog(__FUNCTION__, __LINE__, ss.str());}
#if 1
#define PXR_CHECK_HR_AND_PTR(HR, P, MSG, VAL) if(HR != S_OK || !P) \
    {PXR_TRACE_LOG(MSG << ", error=" << HR); return VAL;}
#define PXR_CHECK_HR(HR, MSG, VAL) if(HR != S_OK) \
    {PXR_TRACE_LOG(MSG << ", error=" << HR); return VAL;}
#else
#define PXR_CHECK_HR_AND_PTR(HR, P, MSG, VAL)
#define PXR_CHECK_HR(HR, MSG, VAL)
#endif


#pragma comment(lib, "Winmm.lib")

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient); 

#define TC_IOCTL(CODE) (CTL_CODE (FILE_DEVICE_UNKNOWN, 0x800 + (CODE), METHOD_BUFFERED, FILE_ANY_ACCESS))
#define TC_IOCTL_GET_DRIVER_VERSION						TC_IOCTL (1)
#define TC_IOCTL_VIRTUALAUDIO_STATUS						TC_IOCTL (2)
#define TC_IOCTL_VIRTUALAUDIO_SEND						TC_IOCTL (3)
#define TC_IOCTL_VIRTUALAUDIO_SETKEY						TC_IOCTL (4)

namespace pxr_base
{
    MicrophoneDevice::MicrophoneDevice(MicrophoneDeviceCallback* callback)
        :callback_(callback)
    {
    }

    MicrophoneDevice::~MicrophoneDevice()
    {
        Stop();
        callback_ = nullptr;
    }

    bool MicrophoneDevice::Start(  bool wireless_mode)
    {
        std::wstring device_name = L"";
		if (!CheckX64System())
		{
			PXR_TRACE_LOG("check x64 system failed");
            callback_->NotifyLogCallback(MicrophoneDeviceCallback::LogLevel::kLogLevelError, "check x64 system failed");
			return false;
		}

		 CoInitialize(nullptr);
		 
		start_flag_ = true;
        if (GetDefaultDevice(origin_defalut_device_id_, origin_defalut_device_name_, EDataFlow::eCapture)==false)
        {
            PXR_TRACE_LOG("get default device failed");
            callback_->NotifyLogCallback(MicrophoneDeviceCallback::LogLevel::kLogLevelError, "get default device failed");
        }
       
        if (wireless_mode)
        {
            device_name = L"Pico Streaming Virtual Audio";
			if (SetDefaultDeviceByName(device_name, EDataFlow::eCapture)==false)
            {
                PXR_TRACE_LOG("set default device to pico failed");
                callback_->NotifyLogCallback(MicrophoneDeviceCallback::LogLevel::kLogLevelError, "set default device to pico failed");
            }
          
			if (!InitVirtualAudioDevice())
			{
				PXR_TRACE_LOG("init audio device failed");
                callback_->NotifyLogCallback(MicrophoneDeviceCallback::LogLevel::kLogLevelError, "init audio device failed");
				Stop();
				return false;
			}
          
        } 
        else
        {
            device_name = L"AC Interface";
			if (SetDefaultDeviceByName(device_name, EDataFlow::eCapture) == false)
			{
				PXR_TRACE_LOG("set default device to pico failed");
				callback_->NotifyLogCallback(MicrophoneDeviceCallback::LogLevel::kLogLevelError, "set default device to pico failed");
			}
        }
        callback_->NotifyLogCallback(MicrophoneDeviceCallback::LogLevel::kLogLevelError, "Start pico mic_phone successed!");
        return true;
    }

    void MicrophoneDevice::Stop()
    {
        callback_->NotifyLogCallback(MicrophoneDeviceCallback::LogLevel::kLogLevelError, "MicrophoneDevice  Stop");
        Reset();
		if (origin_defalut_device_id_.length() > 0)
		{
			int try_index = 0;
			while (try_index < 3)
			{
				if ((SetDefaultDeviceById(origin_defalut_device_id_) == false))
				{
					PXR_TRACE_LOG("set default device failed");
					callback_->NotifyLogCallback(MicrophoneDeviceCallback::LogLevel::kLogLevelError, "origin_defalut_device set failed");
				}
				else
				{
					PXR_TRACE_LOG("set default device ok");
					callback_->NotifyLogCallback(MicrophoneDeviceCallback::LogLevel::kLogLevelError, "set default device ok");
					break;
				}

				if (SetDefaultDeviceByName(origin_defalut_device_name_, EDataFlow::eCapture) == false)
				{
					callback_->NotifyLogCallback(MicrophoneDeviceCallback::LogLevel::kLogLevelError, "origin_defalut_device set failed");
				}
				else
				{
					callback_->NotifyLogCallback(MicrophoneDeviceCallback::LogLevel::kLogLevelError, "set default device ok");
					break;
				}
				Sleep(1000);
				try_index++;

			}
		}
		else
		{
			callback_->NotifyLogCallback(MicrophoneDeviceCallback::LogLevel::kLogLevelError, "origin_defalut_device length is 0");
		}
     
        if (start_flag_)
        {
            start_flag_ = false;
           
        }
       
        CoUninitialize();
         
    }

    bool MicrophoneDevice::Write(char* buff, int len)
    {
        if (vac_handle_ == INVALID_HANDLE_VALUE)
        {
            PXR_TRACE_LOG("VAC not opened");
            return false;
        }

        if (buff == nullptr || len <= 0)
        {
            PXR_TRACE_LOG("invalid buffer, len= " << len);
        }

        DWORD result = 0;
        if (DeviceIoControl(vac_handle_, TC_IOCTL_VIRTUALAUDIO_SEND, buff, len,
            NULL, 0, &result, NULL) != TRUE)
        {
            PXR_TRACE_LOG("write data error= " << GetLastError());
            return false;
        }
        return true;
    }
	bool MicrophoneDevice::SetVolum(int level)
	{
		HRESULT hr;
		IMMDeviceEnumerator* pDeviceEnumerator = 0;
		IMMDevice* pDevice = 0;
		IAudioEndpointVolume* pAudioEndpointVolume = 0;
		IAudioClient* pAudioClient = 0;

		try {
			hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pDeviceEnumerator);
			if (FAILED(hr)) throw "CoCreateInstance";
			hr = pDeviceEnumerator->GetDefaultAudioEndpoint(eCapture, eMultimedia, &pDevice);
			if (FAILED(hr)) throw "GetDefaultAudioEndpoint";
			hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pAudioEndpointVolume);
			if (FAILED(hr)) throw "pDevice->Active";
			hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
			if (FAILED(hr)) throw "pDevice->Active";

			if (level == -2) {
				hr = pAudioEndpointVolume->SetMute(FALSE, NULL);
				if (FAILED(hr)) throw "SetMute";
			}
			else if (level == -1) {
				hr = pAudioEndpointVolume->SetMute(TRUE, NULL);
				if (FAILED(hr)) throw "SetMute";
			}
			else {
				if (level < 0 || level>100) {
					hr = E_INVALIDARG;
					throw "Invalid Arg";
				}

				float fVolume;
				fVolume = level / 100.0f;
				hr = pAudioEndpointVolume->SetMasterVolumeLevelScalar(fVolume, &GUID_NULL);
				if (FAILED(hr)) throw "SetMasterVolumeLevelScalar";

				pAudioClient->Release();
				pAudioEndpointVolume->Release();
				pDevice->Release();
				pDeviceEnumerator->Release();
				return true;
			}
		}
		catch (...) {
			if (pAudioClient) pAudioClient->Release();
			if (pAudioEndpointVolume) pAudioEndpointVolume->Release();
			if (pDevice) pDevice->Release();
			if (pDeviceEnumerator) pDeviceEnumerator->Release();
			throw;
		}
		return false;
	}

    
	bool MicrophoneDevice::GetDefaultDevice(IMMDevice** device, int device_type)
	{
		bool ret = false;
		IMMDeviceEnumerator* pMMDeviceEnumerator = NULL;
		wchar_t wszErrMsg[MAX_PATH] = { 0 };
		HRESULT hr = CoInitialize(NULL);
		
		try
		{
				HRESULT hr = CoCreateInstance(
					__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
					__uuidof(IMMDeviceEnumerator),
					reinterpret_cast<void**>(&pMMDeviceEnumerator)
				);
				if (FAILED(hr))
				{
					wsprintf(wszErrMsg, L"CoCreateInstance(IMMDeviceEnumerator) failed: hr = 0x%08x", hr);
					throw std::exception();
				}

				// get the default render endpoint
				hr = pMMDeviceEnumerator->GetDefaultAudioEndpoint(
					(EDataFlow)device_type, eConsole, device
				);
				if (FAILED(hr))
				{
					wsprintf(wszErrMsg, L"IMMDeviceEnumerator::GetDefaultAudioEndpoint failed: hr = 0x%08x", hr);
					throw std::exception();
				}

				ret = true;
		}
		catch (std::exception&)
		{
				wprintf(wszErrMsg);
		}

		


		{
			if (pMMDeviceEnumerator != nullptr)
			{
				pMMDeviceEnumerator->Release();
			}
		}

		return ret;
	}
	void wcharTochar(const wchar_t* wchar, char* chr, int length)
	{
		WideCharToMultiByte(CP_ACP, 0, wchar, -1,
			chr, length, NULL, NULL);
	}
	bool MicrophoneDevice::GetDefaultDevice(std::wstring& device_id, std::wstring& device_name, int device_type)
	{
		IMMDevice* pMMDevice = nullptr;
		if (!GetDefaultDevice(&pMMDevice, device_type))
		{
			return false;
		}

		bool ret = true;
		wchar_t wszErrMsg[MAX_PATH] = { 0 };
		HRESULT hr;
		IPropertyStore* pPropertyStore = nullptr;
		PROPVARIANT pv;
		PropVariantInit(&pv);

		try
		{
			LPWSTR pwszDeviceId = nullptr;
			hr = pMMDevice->GetId(&pwszDeviceId);
			if (FAILED(hr))
			{
				wsprintf(wszErrMsg, L"IMMDevice::GetId failed: hr = 0x%08x", hr);
				throw;
			}

			// open the property store on that device
			hr = pMMDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
			if (FAILED(hr))
			{
				wsprintf(wszErrMsg, L"IMMDevice::OpenPropertyStore failed: hr = 0x%08x", hr);
				throw;
			}

			// get the long name property
			hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &pv);
			if (FAILED(hr))
			{
				wsprintf(wszErrMsg, L"IPropertyStore::GetValue failed: hr = 0x%08x", hr);
				throw;
			}

			if (VT_LPWSTR != pv.vt)
			{
				wsprintf(wszErrMsg, L"PKEY_Device_FriendlyName variant type is %u - expected VT_LPWSTR", pv.vt);
				throw;
			}

            device_id = pwszDeviceId;
            device_name = pv.pwszVal;
            char deviceid[1024] = {0};
            char devicname[1024] = { 0 };
            wcharTochar(pwszDeviceId, deviceid, 1024);
            wcharTochar(pv.pwszVal, devicname, 1024);
            callback_->NotifyLogCallback(MicrophoneDeviceCallback::LogLevel::kLogLevelError, deviceid);
            callback_->NotifyLogCallback(MicrophoneDeviceCallback::LogLevel::kLogLevelError, devicname);
		}
		catch (std::exception&)
		{
			wprintf(wszErrMsg);
			ret = false;
		}



		if (pMMDevice != nullptr)
		{
			pMMDevice->Release();
		}

		if (pPropertyStore != nullptr)
		{
			pPropertyStore->Release();
		}

		if (pv.vt != VT_EMPTY)
		{
			hr = PropVariantClear(&pv);
			if (FAILED(hr))
			{
				wprintf(L"PropVariantClear failed: hr = 0x%08x", hr);
				ret = false;
			}
		}


		return ret;
	}

	bool MicrophoneDevice::SetDeviceEnable(std::wstring pwszID)
	{
		HRESULT hr;
		IPolicyConfig* _pPolicyConfig = NULL;

		hr = CoCreateInstance(__uuidof(CPolicyConfigClient), NULL, CLSCTX_INPROC, IID_IPolicyConfig2, (LPVOID*)&_pPolicyConfig);

		if (hr != S_OK)
			hr = CoCreateInstance(__uuidof(CPolicyConfigClient), NULL, CLSCTX_INPROC, IID_IPolicyConfig1, (LPVOID*)&_pPolicyConfig);

		if (hr != S_OK)
			hr = CoCreateInstance(__uuidof(CPolicyConfigClient), NULL, CLSCTX_INPROC, IID_IPolicyConfig0, (LPVOID*)&_pPolicyConfig);

		if (hr != S_OK)
			return FALSE;

		hr = _pPolicyConfig->SetEndpointVisibility(pwszID.c_str(), TRUE);

		return (hr == S_OK);
	}

	
    bool MicrophoneDevice::SetDefaultDeviceById(std::wstring& device_id)
    {
		IPolicyConfigVista* pPolicyConfig;
		ERole reserved = eConsole;

		HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient),
			NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID*)&pPolicyConfig);
        if (hr != S_OK)
		hr = CoCreateInstance(__uuidof(CPolicyConfigClient), NULL, CLSCTX_INPROC, IID_IPolicyConfig2, (LPVOID*)&pPolicyConfig);

		if (hr != S_OK)
			hr = CoCreateInstance(__uuidof(CPolicyConfigClient), NULL, CLSCTX_INPROC, IID_IPolicyConfig1, (LPVOID*)&pPolicyConfig);

		if (hr != S_OK)
			hr = CoCreateInstance(__uuidof(CPolicyConfigClient), NULL, CLSCTX_INPROC, IID_IPolicyConfig0, (LPVOID*)&pPolicyConfig);

		if (hr != S_OK)
			return false;
		if (SUCCEEDED(hr))
		{
			hr = pPolicyConfig->SetDefaultEndpoint(device_id.c_str(), reserved);
			reserved = eMultimedia;
			hr = pPolicyConfig->SetDefaultEndpoint(device_id.c_str(), reserved);
			reserved = eCommunications;
			hr = pPolicyConfig->SetDefaultEndpoint(device_id.c_str(), reserved);
			pPolicyConfig->Release();
		}
		else
		{
			return false;
		}
		return true;
    }
    bool MicrophoneDevice::SetDefaultDeviceByName(std::wstring& device_name, int device_type)
    {
		HRESULT hr = S_OK;
		IMMDeviceEnumerator* pEnumerator = NULL;
		IMMDeviceCollection* pCollection = NULL;
		IMMDevice* pEndpoint = NULL;
		IAudioClient* pAudioClient = NULL;
		WAVEFORMATEX* pwfx = NULL;
		IPropertyStore* pProps = NULL;
		LPWSTR pwszID = NULL;
		UINT count = 0;
		DWORD pdwState = 0;
		hr = CoCreateInstance(
			CLSID_MMDeviceEnumerator,
			NULL,
			CLSCTX_ALL,
			IID_IMMDeviceEnumerator,
			(void**)&pEnumerator);

		if (FAILED(hr))
		{
            return false;
		}
		

		hr = pEnumerator->EnumAudioEndpoints(
			(EDataFlow)device_type,
			DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED,
			&pCollection);

		if (FAILED(hr))
		{
			return false;
		}

        bool ret = false;
		hr = pCollection->GetCount(&count);
		if (FAILED(hr))
		{
			return false;
		}

		for (ULONG i = 0; i < count; i++)
		{
			hr = pCollection->Item(i, &pEndpoint);
			if (FAILED(hr))
			{
				return false;
			}

			hr = pEndpoint->GetState(&pdwState);
			if (FAILED(hr))
			{
				return false;
			}

			//wprintf(L"pdwState:%d\r\n", pdwState);

			hr = pEndpoint->GetId(&pwszID);
			if (FAILED(hr))
			{
				return false;
			}

			hr = pEndpoint->OpenPropertyStore(
				STGM_READ,
				&pProps);

			if (FAILED(hr))
			{
				return false;
			}

			PROPVARIANT varName;
			PropVariantInit(&varName);

			hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
			if (FAILED(hr))
			{
				return false;
			}
			std::wstring vrname = varName.pwszVal;
			wprintf(varName.pwszVal);
			wprintf(L"\r\n");

			wprintf(L"pwszID:%s\r\n", pwszID);
            std::wstring device_name_str = varName.pwszVal;
          
			if (device_name_str.find(device_name)!=std::wstring::npos)
			{
				if (pdwState != 0x1)
				{
                  if (SetDeviceEnable(pwszID)==false)
                  {
                      PXR_TRACE_LOG("pico mic set device enable failed");
                  }
				}
                std::wstring id = pwszID;
                ret = SetDefaultDeviceById(id);
				CoTaskMemFree(pwszID);
				pwszID = NULL;
				PropVariantClear(&varName);
				SAFE_RELEASE(pProps);
				SAFE_RELEASE(pEnumerator);
				SAFE_RELEASE(pCollection);
				break;
			}

			CoTaskMemFree(pwszID);
			pwszID = NULL;
			PropVariantClear(&varName);
			SAFE_RELEASE(pProps);
			SAFE_RELEASE(pEndpoint);
		}
        CoTaskMemFree(pwfx);
		SAFE_RELEASE(pEnumerator);
		SAFE_RELEASE(pCollection);
        SAFE_RELEASE(pAudioClient);
		return ret;
  
    }
    bool MicrophoneDevice::CheckX64System()
    {
        SYSTEM_INFO system_info;
        GetNativeSystemInfo(&system_info);
        return system_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
            system_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64;
    }

    bool MicrophoneDevice::InitVirtualAudioDevice()
    {
        if (!OpenVirtualAudioCable())
        {
            PXR_TRACE_LOG("open VAC failed");
            return false;
        }

        LONG drvier_version = 0;
        DWORD result = 0;
        if (DeviceIoControl(vac_handle_, TC_IOCTL_GET_DRIVER_VERSION, NULL, 0,
            &drvier_version, sizeof(drvier_version), &result, NULL) != TRUE)
        {
            PXR_TRACE_LOG("get driver version error= " << GetLastError());
            return false;
        }

        IMMDevice* device = FindPicoAudioDevice();
        if (!device)
        {
            PXR_TRACE_LOG("find pico virtual audio deivce failed");
            return false;
        }
        device_.set(device);

        if (!InitAudioClient())
        {
            PXR_TRACE_LOG("init audio client failed");
            return false;
        }

        DWORD vac_status = 0;
        if (DeviceIoControl(vac_handle_, TC_IOCTL_VIRTUALAUDIO_STATUS, NULL, 0,
            &vac_status, sizeof(vac_status), &result, NULL) != TRUE)
        {
            PXR_TRACE_LOG("get VAC status error=" << GetLastError());
            return false;
        }
        static const DWORD kPicoVirtualAudioKeyLen = 16;
        static const BYTE kPicoVirtualAudioKey[kPicoVirtualAudioKeyLen] = {
            0x0b, 0x8f, 0x71, 0xc1, 0x8f, 0xfa, 0xd9, 0x13,
            0xee, 0x6c, 0xf3, 0xc0, 0xc1, 0xcf, 0xd7, 0x76 };

        if (DeviceIoControl(vac_handle_, TC_IOCTL_VIRTUALAUDIO_SETKEY, 
            const_cast<BYTE*>(kPicoVirtualAudioKey), kPicoVirtualAudioKeyLen,
            NULL, 0, &result, NULL) != TRUE)
        {
            PXR_TRACE_LOG("set key error=" << GetLastError());
            return false;
        }
        return true;
    }

    bool MicrophoneDevice::OpenVirtualAudioCable()
    {
        if (vac_handle_ != INVALID_HANDLE_VALUE)
        {
            PXR_TRACE_LOG("VAC handle existed, handle= " << vac_handle_);
            return false;
        }

        static const auto kVACFilePath = L"\\\\.\\sVirtualAudioCable";
        vac_handle_ = CreateFileW(kVACFilePath, 0,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING,
            0, NULL);
        if (vac_handle_ == INVALID_HANDLE_VALUE)
        {
            PXR_TRACE_LOG("open vac error= " << GetLastError());
            return false;
        }

        return true;
    }

    IMMDevice* MicrophoneDevice::FindPicoAudioDevice()
    {
        AutoReleaser<IMMDeviceEnumerator> enumerator;
        HRESULT hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
            IID_IMMDeviceEnumerator, (void**)&enumerator.get());
        PXR_CHECK_HR(hr, "create enumerator error", nullptr);

        AutoReleaser<IMMDeviceCollection> collection;
        hr = enumerator->EnumAudioEndpoints(eCapture,
            DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED,
            &collection.get());
        PXR_CHECK_HR(hr, "enum audio endpoints", nullptr);

        UINT ept_count = 0;
        hr = collection->GetCount(&ept_count);
        PXR_CHECK_HR(hr, "get device count", nullptr);

        IMMDevice* device = nullptr;
        for (UINT i = 0; i < ept_count; i++)
        {
            AutoReleaser<IMMDevice> check_device;
            hr = collection->Item(i, &check_device.get());
            PXR_CHECK_HR(hr, "get device", nullptr);
            //Note(by Alan Duan): Check device friendly name to find Pico Audio Device.
            if (CheckPropVariantValid(check_device.get()))
            {
                if (!CheckAndSetDeviceVisibility(check_device.get()))
                {
                    PXR_TRACE_LOG("set device visibility failed");
                }
                device = check_device.get();
                check_device.reset();
                break;
            }
        }
        return device;
    }

    bool MicrophoneDevice::InitAudioClient()
    {
        if (!device_)
        {
            PXR_TRACE_LOG("invalid device");
            return false;
        }

        if (audio_client_)
        {
            PXR_TRACE_LOG("audio client existed = " << audio_client_.get());
            return false;
        }

        HRESULT hr = device_->Activate(IID_IAudioClient, CLSCTX_ALL,
            NULL, (void**)&audio_client_.get());
        PXR_CHECK_HR_AND_PTR(hr, audio_client_, "activate audio client", false);
        AutoReleaser<WAVEFORMATEX> wave_format(AutoMemFreeFunc<WAVEFORMATEX>);
        hr = audio_client_->GetMixFormat(&wave_format.get());
        PXR_CHECK_HR_AND_PTR(hr, wave_format, "get mix format", false);
        WAVEFORMATEX set_format;
        set_format.wFormatTag = WAVE_FORMAT_PCM;
        set_format.nChannels = 2;
        set_format.nSamplesPerSec = 48000;
        set_format.nAvgBytesPerSec = 192000;
        set_format.nBlockAlign = 4;
        set_format.wBitsPerSample = 16;
        set_format.cbSize = 0;
        static const REFERENCE_TIME kAudioBufferDuration = 20 * 10000000;
        hr = audio_client_->Initialize(AUDCLNT_SHAREMODE_SHARED,
            0,
            kAudioBufferDuration,
            0,
            &set_format,
            NULL);
        PXR_CHECK_HR(hr, "initialize audio client", false);
        
        hr = audio_client_->GetService(IID_IAudioCaptureClient, (void**)&capture_client_.get());
        PXR_CHECK_HR(hr, "get capture client", false);

        hr = audio_client_->Start();

        return true;
    }

    bool MicrophoneDevice::CheckPropVariantValid(IMMDevice* device)
    {
        AutoReleaser<IPropertyStore> prop_store;
        HRESULT hr = device->OpenPropertyStore(STGM_READ, &prop_store.get());
        PXR_CHECK_HR(hr, "open property store", false);

        PROPVARIANT prop_variant;
        PropVariantInit(&prop_variant);
        hr = prop_store->GetValue(PKEY_Device_FriendlyName, &prop_variant);
        bool res = false;
        static const wchar_t* kPicoVirtualAudioDeviceName = L"Pico Streaming Virtual Audio";
        if (hr == S_OK)
        {
            if (wcsstr(prop_variant.pwszVal, kPicoVirtualAudioDeviceName) != nullptr)
            {
                PXR_TRACE_LOG("find pico device= " << device);
                res = true;
            }
        }
        else
        {
            PXR_TRACE_LOG("get device friendly name error= " << hr);
        }
        PropVariantClear(&prop_variant);
        return res;
    }

    bool MicrophoneDevice::CheckAndSetDeviceVisibility(IMMDevice* device)
    {
        DWORD state = 0;
        HRESULT hr = device->GetState(&state);
        PXR_CHECK_HR(hr, "get device state", false);
        if (state == 0x1)
        {
            return true;
        }
        
        AutoReleaser<WCHAR> device_id(AutoMemFreeFunc<WCHAR>);
        hr = device->GetId(&device_id.get());
        PXR_CHECK_HR(hr, "get device id", false);
        //Todo(by Alan Duan): Trace wchar*.
        PXR_TRACE_LOG("device id= " << device_id.get());

        AutoReleaser<IPolicyConfig> policy_config;
        if (!TryOpenPolicyConfig(policy_config.get()))
        {
            PXR_TRACE_LOG("open policy config failed");
            return false;
        }

        hr = policy_config->SetEndpointVisibility(device_id.get(), TRUE);
        PXR_CHECK_HR(hr, "set device visibility", false);
        return true;
    }
   
    
    bool MicrophoneDevice::TryOpenPolicyConfig(IPolicyConfig*& config)
    {
        static const GUID kPicoPolicyConfigIID[3] =
        {
            IID_IPolicyConfig2,
            IID_IPolicyConfig1,
            IID_IPolicyConfig0
        };

        for (auto& config_id : kPicoPolicyConfigIID)
        {
            HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigClient), NULL,
                CLSCTX_INPROC, config_id, (LPVOID*)&config);
            if (hr == S_OK)
                return true;
        }
        config = nullptr;
        return false;
    }

    void MicrophoneDevice::TraceLog(const char* func, int line, const std::string& msg)
    {
        if (callback_)
        {
            std::stringstream ss;
            ss << "pico ";
            ss << __FUNCTION__ << " " << line << ": ";
            ss << msg;
            callback_->NotifyLogCallback(MicrophoneDeviceCallback::LogLevel::kLogLevelInfo,
                ss.str().c_str());
        }
    }

    void MicrophoneDevice::Reset()
    {
        if (vac_handle_ != INVALID_HANDLE_VALUE)
        {
            CloseHandle(vac_handle_);
            vac_handle_ = INVALID_HANDLE_VALUE;
        }
        if (audio_client_)
        {
            audio_client_->Stop();
        }
        capture_client_.release();
        audio_client_.release();
        device_.release();
    }
}