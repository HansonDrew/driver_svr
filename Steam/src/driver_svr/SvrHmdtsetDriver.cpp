//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#include "SvrHmdtestDriver.h"
#include "driverlog.h"
#include <map>

#include "ConnectionRecorder.h"
#include "D3DHelper.h"
#include "Util.h"
#include "RVRAudioSink.h"
#include "RVRLogger.h"
#include "RVRUtils.h"
#include "PluginManger.h"
using namespace vr;
using namespace RVR;
#include "ConfigReader.h"
extern ConfigReader gConfigReader;
extern PluginManger gPluginManger;
//-----------------------------------------------------------------------------
SvrHmdtestDriver::SvrHmdtestDriver(RVRStub* mStubInstance)
//-----------------------------------------------------------------------------
{
	DriverLog("SvrHmdtestDriver");
	this->mStubInstance = mStubInstance;

	m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
	m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;

	m_nRenderWidth = mStubInstance->GetPluginSettings()->width;
	m_nRenderHeight = mStubInstance->GetPluginSettings()->height;

	m_sSerialNumber.assign("Pico Neo2222");
	m_sModelNumber.assign("Pico Neo2222");
	////modiefy dy dzhuang
	if (gConfigReader.GetControllerType() == 0)
	{
		m_sModelNumber.assign("Pico Neo2222");
	}
	else if (gConfigReader.GetControllerType() == 1)
	{
		m_sModelNumber.assign("Oculus Rift CV1");
	}
	else if (gConfigReader.GetControllerType() == 2)
	{
		m_sModelNumber.assign("VIVE_Pro MV");
	}
	else if (gConfigReader.GetControllerType() == 3)
	{
		m_sModelNumber.assign("Pico VR222");
	}

	//end 


	DriverLog("driver_pico: Serial Number: %s\n", m_sSerialNumber.c_str());
	DriverLog("driver_pico: Model Number: %s\n", m_sModelNumber.c_str());
	DriverLog("driver_pico: Render Target: %d %d\n", m_nRenderWidth, m_nRenderHeight);

	

	poseDataLatest.valid = false;
}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::ClearTextureList()
//-----------------------------------------------------------------------------
{
	auto it = textureSets.begin();
	for (; it != textureSets.end();)
	{
		TextureSet* set = (*it);
		++it;
		delete set;
	}

	textureSets.clear();
}

//-----------------------------------------------------------------------------
SvrHmdtestDriver::~SvrHmdtestDriver()
//-----------------------------------------------------------------------------
{
	if (mStubInstance)
	{
		mStubInstance->ShutDown();
	}

	{
		CloseHandle(textureSetLock);
		CloseHandle(poseHistoryLock);
	}

	ClearTextureList();

	compositor.Destroy();

	if (audioCaptureSession != nullptr)
	{
		audioCaptureSession->Stop();
		delete audioCaptureSession;
		audioCaptureSession = nullptr;
	}

	if (audioSink != nullptr)
	{
		delete audioSink;
		audioSink = nullptr;
	}
}

//-----------------------------------------------------------------------------
vr::EVRInitError SvrHmdtestDriver::Activate(vr::TrackedDeviceIndex_t unObjectId)
//-----------------------------------------------------------------------------
{
	DriverLog("hmdtestactive");
	m_unObjectId = unObjectId;
	m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_unObjectId);


	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ModelNumber_String, m_sModelNumber.c_str());
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RenderModelName_String, m_sModelNumber.c_str());
	vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_UserIpdMeters_Float, mStubInstance->GetPluginSettings()->interPupilDistance);
	vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_DisplayFrequency_Float, (float)(mStubInstance->GetPluginSettings()->fps));
	vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_DoNotApplyPrediction_Bool, true);
	//modeify by dzhuang  2.3.5 版本里保留信息最多（加速，正方向校准。2.3.7有 效果较好的加速和 Oculus 映射原始写法和 出厂厂家等设置）
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RenderModelName_String, "generic_hmd");
	vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/proximity", &mHandelProximity);
	// return a constant that's not 0 (invalid) or 1 (reserved for Oculus)
	vr::VRProperties()->SetUint64Property(m_ulPropertyContainer, Prop_CurrentUniverseId_Uint64, 2019);
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceReady_String, "{pico}/icons/headset_ready.png");
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceOff_String, "{pico}/icons/headset_off.png");
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceStandby_String, "{pico}/icons/headset_standby.png");
	if (gConfigReader.GetControllerType() == 0)//pico
	{
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ManufacturerName_String, "PICO");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_HardwareRevision_String, "V1.0");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_TrackingSystemName_String, "pico");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "rift");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_InputProfilePath_String, "{oculus}/input/rift_profile.json");
	}
	else if (gConfigReader.GetControllerType() == 1)//oculus
	{
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ManufacturerName_String, "Oculus");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_HardwareRevision_String, "V1.0");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_TrackingSystemName_String, "oculus");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "rift");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_InputProfilePath_String, "{oculus}/input/rift_profile.json");
	}
	else if (gConfigReader.GetControllerType() == 2)//htc
	{
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ManufacturerName_String, "HTC");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_HardwareRevision_String, "V1.0");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_TrackingSystemName_String, "lighthouse");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "vive_pro");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_InputProfilePath_String, "{htc}/input/vive_profile.json.json");

	}
	else if (gConfigReader.GetControllerType() == 3)//really pico
	{
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ManufacturerName_String, "PICO");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_HardwareRevision_String, "V1.0");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_TrackingSystemName_String, "pico");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "rift");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_InputProfilePath_String, "{oculus}/input/rift_profile.json");
	}


	//end

	// return a constant that's not 0 (invalid) or 1 (reserved for Oculus)
	//vr::VRProperties()->SetUint64Property( m_ulPropertyContainer, Prop_CurrentUniverseId_Uint64, 2 );

	// avoid "not fullscreen" warnings from vrmonitor
	vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_IsOnDesktop_Bool, false);

	
	return VRInitError_None;
}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::Deactivate()
//-----------------------------------------------------------------------------
{
	m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;

	if (audioCaptureSession)
	{
		audioCaptureSession->Stop();
	}
}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::EnterStandby()
//-----------------------------------------------------------------------------
{
}

//-----------------------------------------------------------------------------
void* SvrHmdtestDriver::GetComponent(const char* pchComponentNameAndVersion)
//-----------------------------------------------------------------------------
{
	void* returnValue = nullptr;

	if (!_stricmp(pchComponentNameAndVersion, vr::IVRDisplayComponent_Version))
	{
		returnValue = (vr::IVRDisplayComponent*)this;
	}

	if (!_stricmp(pchComponentNameAndVersion, vr::IVRDriverDirectModeComponent_Version))
	{
		returnValue = static_cast<vr::IVRDriverDirectModeComponent*>(this);
	}


	return returnValue;
}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
//-----------------------------------------------------------------------------
{
	if (unResponseBufferSize >= 1)
		pchResponseBuffer[0] = 0;
}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::GetWindowBounds(int32_t* pnX, int32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
//-----------------------------------------------------------------------------
{
	*pnX = 0;
	*pnY = 0;
	*pnWidth = m_nRenderWidth * 2;
	*pnHeight = m_nRenderHeight;
}

//-----------------------------------------------------------------------------
bool SvrHmdtestDriver::IsDisplayOnDesktop()
//-----------------------------------------------------------------------------
{
	return false;
}

//-----------------------------------------------------------------------------
bool SvrHmdtestDriver::IsDisplayRealDisplay()
//-----------------------------------------------------------------------------
{
	return false;
}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::GetRecommendedRenderTargetSize(uint32_t* pnWidth, uint32_t* pnHeight)
//-----------------------------------------------------------------------------
{
	*pnWidth = m_nRenderWidth;
	*pnHeight = m_nRenderHeight;
}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::GetEyeOutputViewport(EVREye eEye, uint32_t* pnX, uint32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
//-----------------------------------------------------------------------------
{
	*pnY = 0;
	*pnWidth = m_nRenderWidth;
	*pnHeight = m_nRenderHeight;
	*pnX = ((eEye == Eye_Left) ? 0 : m_nRenderWidth);
}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::GetProjectionRaw(EVREye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom)
//-----------------------------------------------------------------------------
{
#define DEGREES_TO_RADIANS 0.01745329251f

	float halfFovInRadians = this->mStubInstance->GetPluginSettings()->fov * DEGREES_TO_RADIANS * 0.5f;
	float tangent = tan(halfFovInRadians);

	*pfLeft = -tangent;
	*pfRight = tangent;
	*pfTop = -tangent;
	*pfBottom = tangent;
}

//-----------------------------------------------------------------------------
DistortionCoordinates_t SvrHmdtestDriver::ComputeDistortion(EVREye eEye, float fU, float fV)
//-----------------------------------------------------------------------------
{
	DistortionCoordinates_t coordinates;
	coordinates.rfBlue[0] = fU;
	coordinates.rfBlue[1] = fV;
	coordinates.rfGreen[0] = fU;
	coordinates.rfGreen[1] = fV;
	coordinates.rfRed[0] = fU;
	coordinates.rfRed[1] = fV;
	return coordinates;
}

//-----------------------------------------------------------------------------
vr::DriverPose_t SvrHmdtestDriver::GetPose()
//-----------------------------------------------------------------------------
{
	return mLatestPose;
}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::CacheLatestPose(RVR::RVRPoseData* poseData)
//-----------------------------------------------------------------------------
{
	WaitForSingleObject(poseHistoryLock, INFINITE);
	poseDataLatest = *poseData;
	ReleaseMutex(poseHistoryLock);
}

void SvrHmdtestDriver::UpdatePose(RVR::RVRPoseData* data)
{
	if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid)
	{
		if (!data) {
			data = mStubInstance->GetPose();
		}
		ExtractDriverPose(&mLatestPose, data);
		/*DriverLog("HmdData qw=%lf,qx=%lf,qy=%lf,qz=%lf    PositionX=%lf,PositionY=%lf,PositionZ=%lf\n",
			-mLatestPose.qRotation.w, mLatestPose.qRotation.x, mLatestPose.qRotation.y, mLatestPose.qRotation.z,
			-mLatestPose.vecPosition[0], -mLatestPose.vecPosition[1], -mLatestPose.vecPosition[2]);*/

		//add by dzhuang 
		vr::VRDriverInput()->UpdateBooleanComponent(mHandelProximity, true, 0.0);
		pxrConnectionRecorder::GetInstance()->UpdateConnectionStatus(data->valid);
		//end
		vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, mLatestPose, sizeof(DriverPose_t));
	}
}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::RunFrame()
//-----------------------------------------------------------------------------
{
	if (mStubInstance->GetPluginSettings()->poseMode == RVR::RVRPoseMode::RemotePoseFromHMD)
	{
		UpdatePose();
	}
}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::GetNextSwapTextureSetIndex(vr::SharedTextureHandle_t sharedTextureHandles[2], uint32_t(*pIndices)[2])
//-----------------------------------------------------------------------------
{
	{
		(*pIndices)[0]++;
		(*pIndices)[0] %= 3;
		(*pIndices)[1]++;
		(*pIndices)[1] %= 3;
	}

}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::CreateSwapTextureSet(uint32_t unPid, const SwapTextureSetDesc_t* pSwapTextureSetDesc, vr::SharedTextureHandle_t(*pSharedTextureHandles)[3])
//-----------------------------------------------------------------------------
{
	
}

//-----------------------------------------------------------------------------
SvrHmdtestDriver::TextureSet::~TextureSet()
//-----------------------------------------------------------------------------
{
	
}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::DestroySwapTextureSet(vr::SharedTextureHandle_t sharedTextureHandle)
//-----------------------------------------------------------------------------
{
	
}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::DestroyAllSwapTextureSets(uint32_t unPid)
//-----------------------------------------------------------------------------
{
	
}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::SubmitLayer(const SubmitLayerPerEye_t(&perEye)[2], const vr::HmdMatrix34_t* pPose)
//-----------------------------------------------------------------------------
{
	
}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::Present(vr::SharedTextureHandle_t syncTexture)
//-----------------------------------------------------------------------------
{
	
}

//-----------------------------------------------------------------------------
void SvrHmdtestDriver::PostPresent()
//-----------------------------------------------------------------------------
{

	
}


//-----------------------------------------------------------------------------
void SvrHmdtestDriver::ExtractRVRPoseData(RVR::RVRPoseData* poseData, const vr::HmdMatrix34_t* pPose)
//-----------------------------------------------------------------------------
{
	WaitForSingleObject(poseHistoryLock, INFINITE);
	{
		if (!poseDataLatest.valid) {
			poseDataLatest = *mStubInstance->GetPose();
		}
		*poseData = poseDataLatest;
		ExtractRotation(&(poseData->rotation), pPose);
		ExtractPosition(&(poseData->position), pPose);
	}
	ReleaseMutex(poseHistoryLock);
}

