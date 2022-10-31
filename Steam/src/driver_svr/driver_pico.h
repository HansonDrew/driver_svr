#pragma once
#include <openvr_driver.h>
#include "driverlog.h"
#include "d3dhelper.h"
#include "streaming_hmd_driver.h"
#include "controller_driver.h"
//#include "RVRUnityInterface.h"
#include "RVRLogger.h"
#include "dp_hmd_driver.h"
#include "driver_define.h"
#include "gpu_direct_mode.h"
#include"sensor_add_queue.h"
using namespace RVR;
using namespace vr;
// dp  与 wireless driver 合一。
// 1. dp hmd 和 无线 hmd ，哪个先有sensor 先激活哪个
// 2. dp 直接 add pose ，wireless 通过 sensor_manger 管理sensor，在 add thread 中预测 。 add thread 中 依据 dp 是否激活判断是否进行add pose 逻辑
class PicoVRDriver : public IServerTrackedDeviceProvider
{
public:
	virtual EVRInitError Init(IVRDriverContext *pDriverContext);
	virtual void Cleanup();
	virtual const char * const *GetInterfaceVersions() { return vr::k_InterfaceVersions; }
	virtual void RunFrame();
	virtual bool ShouldBlockStandbyMode() { return false; }
	virtual void EnterStandby() {}
	virtual void LeaveStandby() {}
	
	//virtual void OnHMDPose(RVR::RVRPoseHmdData *pose);
	//virtual void OnControllerPose(uint32_t index, RVR::RVRControllerData* pose);
	void AddHmdPose(RVR::RVRPoseHmdData* pose,bool dp=false);
	void AddControllerPose(uint32_t index, RVR::RVRControllerData* pose, HidType::ButtonStateGather button_state = {0});
	static unsigned int __stdcall  SlardarReporterThread (LPVOID lpParameter);
	static unsigned int __stdcall  AddHmdPoseThread(LPVOID lpParameter);
	static unsigned int __stdcall  AddControllerPoseThread(LPVOID lpParameter);
	static unsigned int __stdcall TestSensorThread(LPVOID lpParameter);
	DpHmdDriver* GetDpHmdDriver() { return dp_hmd_; };
	StreamingHmdDriver* GetStreamingHmdDriver() { return streaming_hmd_; };
	ControllerDriver* GetController(int index) { return controller_[index]; };
	int GetHmdActiveType_() { return hmd_active_type_; };
	void SetHmdActiveType_(int type) { hmd_active_type_ = type; };
	sensor_add_queue sensor_queue_;
private:
	//GpuDirectMode *direct_mode_;
	int hmd_active_type_ = Hmd_Active_Type::None;
	ID3D11Device* d3d11Device;
	RVRStub* mStubInstance;
	StreamingHmdDriver* streaming_hmd_ = nullptr;
	DpHmdDriver* dp_hmd_ = nullptr;
	ControllerDriver* controller_[2];
	 
	RVR::RVRPoseMode m_PoseMode;
	std::string driver_folder_;
	static DWORD WINAPI EventPollThread(PVOID);
	void EventPollFunc();
	static DWORD WINAPI PicoPanelProtecterThread(PVOID);
	void PicoPanelProtecterFunc();
	static unsigned int __stdcall  AddPoseTogetherThread(LPVOID lpParameter);
	HANDLE m_hEventThread = INVALID_HANDLE_VALUE;
	HANDLE m_hPicoPanelThread = INVALID_HANDLE_VALUE;

	HANDLE slardar_thread_event_ = INVALID_HANDLE_VALUE;
	HANDLE add_hmd_thread_event_ = INVALID_HANDLE_VALUE;
	HANDLE add_controller_thread_event_[2] = { INVALID_HANDLE_VALUE,  INVALID_HANDLE_VALUE };
	
	HANDLE add_sensor_together_thread_event_ = INVALID_HANDLE_VALUE;
	bool m_bExitEventThread = false;
};
