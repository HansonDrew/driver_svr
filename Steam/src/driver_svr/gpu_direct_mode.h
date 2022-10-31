#pragma once

#include "../../../LiquidVR/include/LiquidVRD2D.h"
 
#include "driver_define.h"


constexpr int kPicoNeo3DisplayVendorId = DisplayEdid;

class GpuDirectMode
{


public:
	bool SetDirectMode(uint32_t vendor_id, bool enable) const;
	bool GetDisplayConnected(uint32_t vendor_id) const;

	GpuDirectMode();
	~GpuDirectMode();

private:
	static bool NvidiaSetDirectMode(uint32_t vendor_id, bool enable);
	static bool NvidiaGetDisplayConnected(uint32_t vendor_id);

	bool AmdInitialize();
	void AmdUnInitialize();
	bool AmdSetDirectMode(uint32_t vendor_id, bool enable) const;
	bool AmdGetDisplayConnected(uint32_t vendor_id) const;

	HMODULE alvr_dll = nullptr;
	ALVRFactory* alvr_factory_ = nullptr;
	ALVRDisplayFactory* alvr_display_factory_ = nullptr;
	ALVRDisplayManager* alvr_display_manager_ = nullptr;
};


