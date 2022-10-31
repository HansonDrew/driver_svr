//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#pragma once

#include <openvr_driver.h>
#include <stdint.h>
#include <thread>
#include <list>
#include "RVRStub.h"
#include "Compositor.h"
#include "AudioCaptureSession.h"
#include "SvrControllerDriver.h"
#include"SvrHmdtestDriver.h"
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class SvrHmdtestDriver : public vr::ITrackedDeviceServerDriver, public vr::IVRDisplayComponent, public vr::IVRDriverDirectModeComponent
{
public:
	SvrHmdtestDriver(RVRStub* stub);
	virtual ~SvrHmdtestDriver();
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
    virtual void CreateSwapTextureSet(uint32_t unPid, const SwapTextureSetDesc_t *pSwapTextureSetDesc, vr::SharedTextureHandle_t(*pSharedTextureHandles)[3]) override;
    virtual void DestroySwapTextureSet(vr::SharedTextureHandle_t sharedTextureHandle) override;
	virtual void DestroyAllSwapTextureSets(uint32_t unPid) override;
    virtual void SubmitLayer(const SubmitLayerPerEye_t(&perEye)[2], const vr::HmdMatrix34_t *pPose) override;
    virtual void Present(vr::SharedTextureHandle_t syncTexture) override;
    virtual void PostPresent() override;
    virtual void GetNextSwapTextureSetIndex(vr::SharedTextureHandle_t sharedTextureHandles[2], uint32_t(*pIndices)[2]) override;
public:
    void RunFrame();
    std::string GetSerialNumber() const { return m_sSerialNumber; }
    void UpdatePose(RVR::RVRPoseData *data = nullptr);
private:
    void ExtractRVRPoseData(RVR::RVRPoseData* poseData, const vr::HmdMatrix34_t *pPose);
    void CacheLatestPose(RVR::RVRPoseData* poseData);

private:
	vr::TrackedDeviceIndex_t m_unObjectId;
	vr::PropertyContainerHandle_t m_ulPropertyContainer;
	std::string m_sSerialNumber;
	std::string m_sModelNumber;
	int32_t m_nRenderWidth;
	int32_t m_nRenderHeight;
	//add by dzhuang
	vr::VRInputComponentHandle_t 	mHandelProximity;
	//end
    ID3D11Device * d3d11Device;
    RVRStub* mStubInstance;

    vr::DriverPose_t mLatestPose;
    RVR::RVRPoseData mCurrentRenderPose;
    Compositor compositor;
    ID3D11Texture2D* depthTexture[2];

    //Audio
    AudioCaptureSession* audioCaptureSession;
    AudioCaptureSession::AudioSink* audioSink;

    //Pose History
    struct PoseHistoryEntry {
        RVR::RVRPoseData poseData;
    };
    RVR::RVRPoseData poseDataLatest;
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
    int64_t startRenderTime;
private:
    void ClearTextureList();

    enum Eye {
        LEFT = 0,
        RIGHT = 1
    };
};
