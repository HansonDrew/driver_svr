#pragma once
#include <vector>
#include <time.h>

#include "openvr_driver.h"
#include "driverlog.h"
#include "driver_define.h"
#include "../RVRPluginDefinitions.h"
#include"audio_capture_session.h"
#include "base_hmd.h"
#include "distortion_dp.h"
#define DEGREES_TO_RADIANS 0.01745329251f
using namespace vr;
#ifdef NO_DP
class DpHmdDriver : public BaseHmd
#else
class DpHmdDriver : public  ITrackedDeviceServerDriver, public  IVRDisplayComponent, public BaseHmd
#endif

{
public:
	DpHmdDriver( );
	~DpHmdDriver();
	AudioCaptureSession* audioCaptureSession;
	int32_t edid_vendor_id_ = DisplayEdid;// 0x2d40;// 0x2e4f;//0x2e4f;//0xd242;
	// internal interface
	bool Init();
	bool EnableNvidiaDp();
	bool EnableAmdDp();
	void Cleanup();
	void RunFrame();
#ifdef NO_DP
	// ITrackedDeviceServerDriver interface
	virtual EVRInitError Activate(uint32_t unObjectId) ;
	virtual void Deactivate() ;
	virtual void EnterStandby() ;
	void* GetComponent(const char* pchComponentNameAndVersion) ;
	virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) ;
	virtual DriverPose_t GetPose() ;
	//////////////////
	// IVRDisplayComponent interface
	virtual void GetWindowBounds(int32_t* pnX, int32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight) ;
	virtual bool IsDisplayOnDesktop() ;
	virtual bool IsDisplayRealDisplay() ;
	virtual void GetRecommendedRenderTargetSize(uint32_t* pnWidth, uint32_t* pnHeight) ;
	virtual void GetEyeOutputViewport(EVREye eEye, uint32_t* pnX, uint32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight) ;
	virtual void GetProjectionRaw(EVREye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom) ;
	virtual DistortionCoordinates_t ComputeDistortion(EVREye eEye, float fU, float fV) ;
	//////////////////	

#else
	// ITrackedDeviceServerDriver interface
	virtual EVRInitError Activate(uint32_t unObjectId) override;
	virtual void Deactivate() override;
	virtual void EnterStandby() override;
	void* GetComponent(const char* pchComponentNameAndVersion) override;
	virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;
	virtual DriverPose_t GetPose() override;
	//////////////////
	// IVRDisplayComponent interface
	virtual void GetWindowBounds(int32_t* pnX, int32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight) override;
	virtual bool IsDisplayOnDesktop() override;
	virtual bool IsDisplayRealDisplay() override;
	virtual void GetRecommendedRenderTargetSize(uint32_t* pnWidth, uint32_t* pnHeight) override;
	virtual void GetEyeOutputViewport(EVREye eEye, uint32_t* pnX, uint32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight) override;
	virtual void GetProjectionRaw(EVREye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom) override;
	virtual DistortionCoordinates_t ComputeDistortion(EVREye eEye, float fU, float fV) override;
	//////////////////	

#endif
	
	
	 
 
	void HmdMatrix_SetIdentity(vr::HmdMatrix34_t* pMatrix);
	vr::HmdMatrix34_t m_eyeToHeadLeft;
	vr::HmdMatrix34_t m_eyeToHeadRight;
	void ReSetPose();
	void UpdatePose(RVR::RVRPoseHmdData* data=nullptr);
	 
#ifdef DIRECTHID
	 
#endif
	float GetPredicted_() { return predicted_; };
	void SetIpd_(float ipd) 
	{		
		ipd_ = ipd;		
	};
	float GetIpd_() { return ipd_; };
	uint32_t GetObjectId_() { return vr_device_index_; };
	int64_t GetTimestamp0fPose() { return timestamp_of_pose_; };
	void SetTimestamp0fPose(int64_t timestamp) { timestamp_of_pose_ = timestamp; };
	bool GetIsActived_() { return is_active_; }
	void SetIsAdded_(bool add) { is_add_ = add; };
	bool GetIsAdded_() { return is_add_; };
	vr::DriverPose_t driver_pose_;
    std::string GetSerialNumber_() { return serial_number_; };
	std::string GetModeNumber_() { return model_number_; };
	void SetOffsetIpd_(float ipd_, float offset_ul, float offset_vl, float offset_ur, float offset_vr);
	void GetOffsetIpd_(float ipd_, float& offset_ul, float& offset_vl, float& offset_ur, float& offset_vr);
	void SetHmdType(int type) { hmd_type_ = type; };
	int GetHmdType() { return hmd_type_; };
	static unsigned int __stdcall  DpStateReporterThread(LPVOID lpParameter);
	HANDLE report_thread_event_ = INVALID_HANDLE_VALUE;

	static unsigned int __stdcall  DpStateCheckThread(LPVOID lpParameter);
	HANDLE check_thread_event_ = INVALID_HANDLE_VALUE;
	bool GetLoop_() { return loop_; };
	void SetLoop_(bool loop) { loop_ = loop; };
	int GetDpRenderFlag_() { return dp_render_flag_; };
	void SetDpRenderFlag_(int flag) { dp_render_flag_ = flag; };
	SOCKET report_socket_;
private:
	int dp_render_flag_ = 0;
	bool loop_=false;
	vr::VRInputComponentHandle_t 	mHandelProximity;
	int hmd_type_ = PRO;
	float offset_ipd_min_l_[2] = { 0.0f,0.0f };
	float offset_ipd_mid_l_[2] = { 0.0f,0.0f };
	float offset_ipd_max_l_[2] = { 0.0f,0.0f };
	float offset_ipd_min_r_[2] = { 0.0f,0.0f };
	float offset_ipd_mid_r_[2] = { 0.0f,0.0f };
	float offset_ipd_max_r_[2] = { 0.0f,0.0f };
	bool is_active_=false;
	bool is_add_ = false;
	int64_t timestamp_of_pose_;
	void SetProperties();
	float render_scale_;
	float predicted_ = -10;
	float predicted_addvalue_=-1;
	float ipd_= LENS_SEPARATION;
	
	TrackedDeviceIndex_t vr_device_index_;

	std::string  serial_number_;
	std::string model_number_;
	float h_screen_size_  ;
	float v_screen_size_ ;
	int32_t window_x_;
	int32_t window_y_;
	int32_t window_width_;
	int32_t window_height_;
	int32_t render_width_;
	int32_t render_height_;
	float seconds_from_vsync_to_photons_;
	float display_frequency_;
	float m_fMetersPerTanAngle;

};

