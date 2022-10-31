#include "gpu_direct_mode.h"

#include "driverlog.h"
#include "../../../LiquidVR/include/LiquidVRD2D.h"
#include "nvapi.h"



bool GpuDirectMode::SetDirectMode(uint32_t vendor_id, bool enable) const
{
	return NvidiaSetDirectMode(vendor_id, enable) || AmdSetDirectMode(vendor_id, enable);
}

bool GpuDirectMode::GetDisplayConnected(uint32_t vendor_id) const
{
	return NvidiaGetDisplayConnected(vendor_id) || AmdGetDisplayConnected(vendor_id);
}

GpuDirectMode::GpuDirectMode()
{
	if (!AmdInitialize())
	{
		DriverLog("[Runtime][GpuDirectMode]Try to init AMD display with failure. If you don't have AMD GPU, please ignore it, Otherwise please confirm the driver is correct installed.");
	}
}

GpuDirectMode::~GpuDirectMode()
{
	AmdUnInitialize();
}

bool GpuDirectMode::NvidiaSetDirectMode(uint32_t vendor_id, bool enable)
{
	NV_CHIPSET_INFO info;
	info.version = NV_CHIPSET_INFO_VER;
	auto status = NvAPI_SYS_GetChipSetInfo(&info);
	if (NVAPI_OK != status)
	{
		return false;
	}

	status = enable ? NvAPI_DISP_EnableDirectMode(vendor_id, 0)
		: NvAPI_DISP_DisableDirectMode(vendor_id, 0);
	if (NVAPI_OK != status)
	{
		return false;
	}

	return true;
}

bool GpuDirectMode::NvidiaGetDisplayConnected(uint32_t vendor_id)
{
	//TEST
	NvAPI_Status r = NVAPI_OK;
	NvU32 deviceCount = 0;
	NV_DISPLAYCONFIG_PATH_INFO_V2* pathInfo = NULL;


	NvU32 displays = 0;
	const auto status = NvAPI_DISP_EnumerateDirectModeDisplays(vendor_id, &displays, nullptr, NV_ENUM_DIRECTMODE_DISPLAY_ENABLED);
	if (status != NVAPI_OK)
	{
		return false;
	}

	return displays != 0;
}

bool GpuDirectMode::AmdInitialize()
{

	if (alvr_display_manager_ != nullptr)
	{
		return true;
	}

	alvr_dll = LoadLibraryW(ALVR_DLL_NAME);
	if (alvr_dll == nullptr)
	{
		return false;
	}

	const auto alvr_init_func = (ALVRInit_Fn)GetProcAddress(alvr_dll, ALVR_INIT_FUNCTION_NAME);
	if (alvr_init_func == nullptr)
	{
		return false;
	}

	const auto alvr_display_init_func = (ALVRDisplayInit_Fn)GetProcAddress(alvr_dll, ALVR_DISPLAY_INIT_FUNCTION_NAME);
	if (alvr_display_init_func == nullptr)
	{
		return false;
	}

	auto alvr_res = alvr_init_func(ALVR_FULL_VERSION, (void**)&alvr_factory_);
	if (ALVR_OK != alvr_res)
	{
		return false;
	}

	alvr_res = alvr_display_init_func(ALVR_FULL_VERSION, (void**)&alvr_display_factory_);
	if (ALVR_OK != alvr_res)
	{
		return false;
	}

	alvr_res = alvr_display_factory_->CreateManager(NULL, &alvr_display_manager_);
	if (ALVR_OK != alvr_res)
	{
		return false;
	}


	return true;
}

void GpuDirectMode::AmdUnInitialize()
{
	if (alvr_display_manager_ != nullptr)
	{
		alvr_display_manager_->Release();
		alvr_display_manager_ = nullptr;
	}

	alvr_display_factory_ = nullptr;
	alvr_factory_ = nullptr;

	if (alvr_dll != nullptr)
	{
		::FreeLibrary(alvr_dll);
		alvr_dll = nullptr;
	}
}

bool GpuDirectMode::AmdSetDirectMode(uint32_t vendor_id, bool enable) const
{
	if (alvr_display_manager_ == nullptr)
	{
		return false;
	}

	ALVR_RESULT alvr_res;
	if (enable)
	{
		alvr_res = alvr_display_manager_->RegisterHiddenDisplay(vendor_id, 0, 0xffff);
	}
	else
	{
		alvr_res = alvr_display_manager_->UnregisterHiddenDisplay(vendor_id, 0, 0xffff);
	}

	if (ALVR_OK != alvr_res)
	{
		return false;
	}

	return true;
}

bool GpuDirectMode::AmdGetDisplayConnected(uint32_t vendor_id) const
{
	if (alvr_display_manager_ == nullptr)
	{
		return false;
	}

	bool connected = false;

	ALVRDisplayInfoEnumerator* enumerator = nullptr;
	const auto alvr_res = alvr_display_manager_->EnumerateDisplays(ALVR_DISPLAY_VISIBILITY_PERMANENTLY_HIDDEN, &enumerator);
	if (alvr_res != ALVR_OK)
	{
		return false;
	}

	while (true)
	{
		ALVRDisplayInfo* display_info = nullptr;
		enumerator->Next(&display_info);
		if (display_info == nullptr)
		{
			break;
		}

		if (display_info->GetManufacturerName() == vendor_id)
		{
			connected = true;
		}

		display_info->Release();
	}

	enumerator->Release();

	return connected;
}

