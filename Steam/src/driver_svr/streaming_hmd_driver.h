//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#pragma once
 
#include <Windows.h>
#include <openvr_driver.h>
#include <stdint.h>
#include <thread>
#include <list>
#include "RVRStub.h"
#include "Compositor.h"
#include "audio_capture_session.h"
#include "controller_driver.h"
#include "RGBToNV12ConverterD3D11.h"
#include "RgbToNV12.h"
#include "base_hmd.h"
#include "../RVRPlugin/IVEncPlugin.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

class StreamingHmdDriver : public vr::ITrackedDeviceServerDriver, public vr::IVRDisplayComponent, public vr::IVRDriverDirectModeComponent,public BaseHmd
{
public:
	static unsigned int __stdcall DepthCalcThread(LPVOID lpParameter);
	static unsigned int __stdcall FunctionDebugThread(LPVOID lpParameter);
	
	StreamingHmdDriver(RVRStub* stub, ID3D11Device* d3d11);
	virtual ~StreamingHmdDriver();
	unsigned int mFunctionDebugThreadId;
	unsigned int mDepthCalcThreadId;
	bool mLoop = false;
	bool change_gop_=false;
	int idr_immediately_ = 0;
	HANDLE depthcalc_thread_event_ = INVALID_HANDLE_VALUE;
	HANDLE function_debug_thread_event_ = INVALID_HANDLE_VALUE;
	HANDLE split_encode_thread_event_ = INVALID_HANDLE_VALUE;
	void Cleanup();
public: //ITrackedDeviceServerDriver
	virtual vr::EVRInitError Activate(vr::TrackedDeviceIndex_t unObjectId) override;
	virtual void Deactivate() override;
	virtual void EnterStandby() override;
    virtual void *GetComponent(const char *pchComponentNameAndVersion) override;
    virtual void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize) override;
    virtual vr::DriverPose_t GetPose() override;
public: //IVRDisplayComponent
    virtual void GetWindowBounds(int32_t *pnX, int32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight);
	virtual bool IsDisplayOnDesktop();
	virtual bool IsDisplayRealDisplay();
	virtual void GetRecommendedRenderTargetSize(uint32_t *pnWidth, uint32_t *pnHeight);
	virtual void GetEyeOutputViewport(vr::EVREye eEye, uint32_t *pnX, uint32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight);
	virtual void GetProjectionRaw(vr::EVREye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom);
	virtual vr::DistortionCoordinates_t ComputeDistortion(vr::EVREye eEye, float fU, float fV);
public: //IVRDriverDirectModeComponent
   // virtual void CreateSwapTextureSet(uint32_t unPid, const SwapTextureSetDesc_t *pSwapTextureSetDesc, vr::SharedTextureHandle_t(*pSharedTextureHandles)[3]) override;
    virtual void DestroySwapTextureSet(vr::SharedTextureHandle_t sharedTextureHandle) override;
	virtual void DestroyAllSwapTextureSets(uint32_t unPid) override;
    //virtual void SubmitLayer(const SubmitLayerPerEye_t(&perEye)[2], const vr::HmdMatrix34_t *pPose) override;
	virtual void CreateSwapTextureSet(uint32_t unPid, const SwapTextureSetDesc_t *pSwapTextureSetDesc, SwapTextureSet_t *pOutSwapTextureSet) override;
	virtual void SubmitLayer(const SubmitLayerPerEye_t(&perEye)[2]) override;
	virtual void Present(vr::SharedTextureHandle_t syncTexture) override;
    virtual void PostPresent() override;
    virtual void GetNextSwapTextureSetIndex(vr::SharedTextureHandle_t sharedTextureHandles[2], uint32_t(*pIndices)[2]) override;
	virtual void GetFrameTiming(vr::DriverDirectMode_FrameTiming* pFrameTiming)  override;
public:
	int64_t last_create_texture_ts = 0;
	void StartEncoder();
	int  GetEncodeFrameConfig(int64_t CopyStart);
	void SetIcons();
	bool GetIsActived_() { return is_actived_; };
	void SetIsActived_(bool is_actived) { is_actived_ = is_actived; }
	bool GetIsAdded_() { return is_added_; };
	void SetIsAdded_(bool is_added) { is_added_ = is_added; }
	float GetIpd_() { return ipd_; };
	void SetIpd_(float ipd) { ipd_ = ipd; };
	int  message_loop_time_;
    void RunFrame();
    std::string GetSerialNumber() const { return m_sSerialNumber; }
    void UpdatePose(RVR::RVRPoseHmdData*data = nullptr);
	//void UpdatePose(RVR::RVRPoseHmdData* data = nullptr);
	void Operatie_Additional(RVR::RVRPoseHmdData* data = nullptr);
	RVR::RVRPoseHmdData split_poses_[SplitTextureSize];
#define LISTNUM 5
	RVR::RVRPoseHmdData pose_list_[LISTNUM];
	int pose_list_index_ = 0;
	int64_t lastPresent = 0; 
	int64_t endPresent = 0;
	int64_t presentOutTs = 0;
	CRITICAL_SECTION cs_pose_;
	void AddPoseList(RVR::RVRPoseHmdData *pose);
	bool FindAndDelete(RVR::RVRPoseHmdData pose_in, RVR::RVRPoseHmdData& retur_value_pose);
	Compositor* GetCompositor() { return (Compositor*)&compositor; };
	void SetDetph_(float depth_value)
	{
		EnterCriticalSection(&cs_depth_);
		depth_ = depth_value;
		LeaveCriticalSection(&cs_depth_);
	}
	float GetDetph_()
	{
		float depth_value = 0;
		EnterCriticalSection(&cs_depth_);
		depth_value = depth_;
		LeaveCriticalSection(&cs_depth_);
		return depth_value;
	}
	void ExtractDriverPose(RVR::RVRPoseHmdData* data);

	void ReSetPose();
	int GetFrameIndex_() { return frame_index_; };
	RVR::RVRPoseHmdData GetCurrentPose();
	void SetCurrentPose(RVR::RVRPoseHmdData hmd_pose);
	vr::TrackedDeviceIndex_t GetVrDeviceId() { return vr_device_index_; };
	RVR::RVRPoseHmdData GetLastRenderPose()
	{
		return last_render_pose;
	};
	void SetLastRenderPose(RVR::RVRPoseHmdData hmd_pose);
	
private:
	RVR::RVRPoseHmdData last_render_pose;
	std::mutex current_pose_mutex_;
	bool left_encoder_init_ = false;
	bool right_encoder_init_ = false;
    void ExtractRVRPoseHmdData(RVR::RVRPoseHmdData* poseData, const vr::HmdMatrix34_t *pPose);
    void CacheLatestPose(RVR::RVRPoseHmdData* poseData);
	float ipd_;
	int depth_inedx_=0;
	int64_t per_frame_ns_ = 0;	
	int frame_index_=0;
	static unsigned int __stdcall split_encode_thread_(LPVOID lpParameter);
	ID3D11Device* GetD3D11Device() { return d3d11device_; };
	ID3D11Device* GetSplitD3D11Device() { return split_d3d11device_; };
	ID3D11Texture2D* GetSplitToEncodeTexture_() { return split_to_encode_texture_; };
	
private:
	ID3D11Texture2D* split_to_encode_texture_ = nullptr;
	int64_t hmd_active_timestamp_;
	bool is_added_ = false;
	bool is_actived_ = false;
	vr::TrackedDeviceIndex_t vr_device_index_;
	vr::PropertyContainerHandle_t m_ulPropertyContainer;
	std::string m_sSerialNumber;
	std::string m_sModelNumber;
	int32_t m_nRenderWidth;
	int32_t m_nRenderHeight;
	CRITICAL_SECTION cs_depth_;
	float depth_;
	//add by dzhuang
	vr::VRInputComponentHandle_t 	mHandelProximity;
	//end
    ID3D11Device * d3d11device_;
	ID3D11Device* split_d3d11device_;
    RVRStub* mStubInstance;
	std::mutex driver_pose_mutex_;
    vr::DriverPose_t driver_pose_;
    RVR::RVRPoseHmdData mCurrentRenderPose;
    Compositor compositor;
    ID3D11Texture2D* depthTexture[2];

    //Audio
    AudioCaptureSession* audioCaptureSession;
   

    //Pose History
    struct PoseHistoryEntry {
        RVR::RVRPoseHmdData poseData;
    };
    RVR::RVRPoseHmdData poseDataLatest;
    HANDLE poseHistoryLock;

    // Texture Sets
	class TextureSet {
	public:
		ID3D11Texture2D* texture[3];
		HANDLE handle[3];
		uint32_t unPid;
	public:
		~TextureSet();
	};
	std::list<TextureSet*> textureSets;
    HANDLE textureSetLock; //TODO: Required ?
    int64_t startFrameTime;
	int64_t lastFrameTime=0;

	int64_t picoRenderStart;
	int64_t  picoRenderEnd = 0;
	int64_t  picoRenderCostUs = 0;
	RVR::VEncFrameConfig  mFrameConfig[3];
	int  mFrameConfigIndex = 0;
private:
    void ClearTextureList();

   
};
