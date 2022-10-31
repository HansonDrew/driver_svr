#include "AudioDeviceManger.h"

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
#include "pico_config.h"
#include <endpointvolume.h>
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
	      	      { (punk)->Release(); (punk) = NULL; }

#pragma comment(lib, "Winmm.lib")


const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

bool AudioDeviceManger::GetDefaultDevice(IMMDevice** device, int device_type)
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
bool AudioDeviceManger::GetDefaultDevice(std::wstring& device_id, std::wstring& device_name, int device_type)
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

bool AudioDeviceManger::SetDeviceEnable(std::wstring pwszID)
{
	HRESULT hr;
	IPicoConfig* _pPicoConfig = NULL;

	hr = CoCreateInstance(__uuidof(CPicoConfigClient), NULL, CLSCTX_INPROC, IID_IPicoConfig2, (LPVOID*)&_pPicoConfig);

	if (hr != S_OK)
		hr = CoCreateInstance(__uuidof(CPicoConfigClient), NULL, CLSCTX_INPROC, IID_IPicoConfig1, (LPVOID*)&_pPicoConfig);

	if (hr != S_OK)
		hr = CoCreateInstance(__uuidof(CPicoConfigClient), NULL, CLSCTX_INPROC, IID_IPicoConfig0, (LPVOID*)&_pPicoConfig);

	if (hr != S_OK)
		return FALSE;

	hr = _pPicoConfig->SetEndpointVisibility(pwszID.c_str(), TRUE);

	return (hr == S_OK);
}


bool AudioDeviceManger::SetDefaultDeviceById(std::wstring& device_id)
{
	IPicoConfigVista* pPicoConfig;
	ERole reserved = eConsole;

	HRESULT hr = CoCreateInstance(__uuidof(CPicoConfigVistaClient),
		NULL, CLSCTX_ALL, __uuidof(IPicoConfigVista), (LPVOID*)&pPicoConfig);
	if (SUCCEEDED(hr))
	{
		hr = pPicoConfig->SetDefaultEndpoint(device_id.c_str(), reserved);
		reserved = eMultimedia;
		hr = pPicoConfig->SetDefaultEndpoint(device_id.c_str(), reserved);
		reserved = eCommunications;
		hr = pPicoConfig->SetDefaultEndpoint(device_id.c_str(), reserved);
		pPicoConfig->Release();
	}
	else
	{
		return false;
	}
	return true;
}
bool AudioDeviceManger::SetDefaultDeviceByName(std::wstring& device_name, int device_type)
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

		if (device_name_str.find(device_name) != std::wstring::npos)
		{
			if (pdwState != 0x1)
			{
				if (SetDeviceEnable(pwszID) == false)
				{
					printf("pico mic set device enable failed");
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


bool AudioDeviceManger::StartUp() 
{
	if (GetDefaultDevice(origin_defalut_device_id_, origin_defalut_device_name_, EDataFlow::eCapture) == false)
	{
		return false; 
	}
	return true;
}

bool AudioDeviceManger::ClearUp()
{
	if (origin_defalut_device_id_.length()>0)
	{
		int try_index = 0;
		while (try_index < 3)
		{
			if ((SetDefaultDeviceById(origin_defalut_device_id_) == false))
			{
				printf("set default device failed");
				 
			}
			else
			{

				return true;
			}
			if (SetDefaultDeviceByName(origin_defalut_device_name_, EDataFlow::eCapture) == false)
			{
				printf("set default device failed");
			}
			else
			{
				return true;
			}
			Sleep(1000);
			try_index++;

		}
	}
	return false;
	
}