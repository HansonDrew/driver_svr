#include "SensorSocket.h"
#include "dp_hmd_driver.h"
#include <process.h>
#include <openvr_driver.h>
#include "Util.h"
#include "driverlog.h"
#include "driver_define.h"
#include "steamvr_tool.h"
#include "config_reader.h"
#include "TimeTool.h"
#include "hid_module.h"
#include "Util.h"
#include "RVRLogger.h"
using namespace RVR;
extern ConfigReader gConfigReader;
extern bool gIsDebug ;
extern bool gLogOutHmdPose ;
extern bool gLog ;
DpHmdDriver::DpHmdDriver()
	: vr_device_index_(vr::k_unTrackedDeviceIndexInvalid)
	, serial_number_("Pico Neo3"),
	model_number_("Pico Neo3")
{
	
	DriverLog("DpHmdDriver::DpHmdDriver()\n");
	Init();
}

DpHmdDriver::~DpHmdDriver()
{
	//loop_ = false;	 
	DriverLog("DpHmdDriver::~DpHmdDriver()\n");
	 
}


 
// internal interface
bool DpHmdDriver::Init()
{

	 if (EnableNvidiaDp()||EnableAmdDp()==false)
	 {
	 	DriverLog("nvidai dp mode and amd dp mode all enable unsucessfully\n");
	 }
	 if (gConfigReader.GetControllerType() == 0|| gConfigReader.GetControllerType() == 3|| gConfigReader.GetControllerType() == 4)
	 {
		 serial_number_.assign("Pico Neo 3");
		 model_number_.assign("Pico Neo 3");
	 }
	 else if (gConfigReader.GetControllerType() == 1)
	 {
		 serial_number_.assign("WMHD316Q512X13");
		 model_number_.assign("Oculus Rift CV1");
	 }
	 else if (gConfigReader.GetControllerType() == 2)
	 {
		 serial_number_.assign("LHR - 6FF22567");
		 model_number_.assign("VIVE_Pro MV");
	 }
	 HidModule::GetInstance()->StartUp();
	 ipd_ = LENS_SEPARATION;
	 h_screen_size_ = WIDTH_METERS;
	 v_screen_size_ = HEIGHT_METERS;
	 window_width_ = WIDTH_PIXELS;
	 window_height_ = HEIGHT_PIXELS;
	 display_frequency_ = float(gConfigReader.GetFps());
	 m_fMetersPerTanAngle = METERS_PER_TAN_ANGLE_AT_CENTER;

	 window_x_ = 0;
	 window_y_ = 0;

	 DriverLog("dp::Init : window_width_=%d, window_height_=%d, window_x_=%d, window_y_=%d\n", window_width_, window_height_, window_x_, window_y_);
	 render_scale_ = 1.2;
	 DriverLog(" dp: %f\n", render_scale_);
	 render_width_ = (uint32_t)((float)window_width_ / 2 * render_scale_);
	 render_height_ = (uint32_t)((float)window_height_ * render_scale_);
 
	 DriverLog("driver_pico: Serial Number: %s\n", serial_number_.c_str());
	 DriverLog("driver_pico: Model Number: %s\n", model_number_.c_str());
	 DriverLog("driver_pico: Window: %d %d %d %d\n", window_x_, window_y_, window_width_, window_height_);
	 DriverLog("driver_pico: Render Target: %d %d, fScale: %f\n", render_width_, render_height_, render_scale_);
	 DriverLog("driver_pico: Seconds from Vsync to Photons: %f\n", seconds_from_vsync_to_photons_);
	 DriverLog("driver_pico: Display Frequency: %f\n", display_frequency_);
	 DriverLog("driver_pico: IPD: %f\n", ipd_);

	 return true;
}

bool DpHmdDriver::EnableNvidiaDp()
{
	return true;
}

bool DpHmdDriver::EnableAmdDp()
{
	return true;

}
 
void DpHmdDriver::ReSetPose()
{
	driver_pose_.poseTimeOffset = 0;

	for (int i = 0; i < 3; i++)
	{
		driver_pose_.vecWorldFromDriverTranslation[i] = 0.0;
		driver_pose_.vecDriverFromHeadTranslation[i] = 0.0;
	}

	driver_pose_.qRotation.w = 1;
	driver_pose_.qRotation.x = 0;
	driver_pose_.qRotation.y = 0;
	driver_pose_.qRotation.z = 0;

	driver_pose_.qWorldFromDriverRotation.w = 1;
	driver_pose_.qWorldFromDriverRotation.x = 0;
	driver_pose_.qWorldFromDriverRotation.y = 0;
	driver_pose_.qWorldFromDriverRotation.z = 0;

	driver_pose_.qDriverFromHeadRotation.w = 1;
	driver_pose_.qDriverFromHeadRotation.x = 0;
	driver_pose_.qDriverFromHeadRotation.y = 0;
	driver_pose_.qDriverFromHeadRotation.z = 0;

	// some things are always true
	driver_pose_.shouldApplyHeadModel = false;// true;
	driver_pose_.willDriftInYaw = false;

	// we don't do position, so these are easy
	for (int i = 0; i < 3; i++)
	{
		driver_pose_.vecPosition[i] = 0.0;
		driver_pose_.vecVelocity[i] = 0.0;
		driver_pose_.vecAcceleration[i] = 0.0;

		// we also don't know the angular velocity or acceleration
		driver_pose_.vecAngularVelocity[i] = 0.0;
		driver_pose_.vecAngularAcceleration[i] = 0.0;

	}
}
  
void DpHmdDriver::Cleanup()
{
	closesocket(report_socket_);
	loop_ = false;
	if (is_active_)
	{
		OnClearUp();
		WaitForSingleObject(check_thread_event_, INFINITE);
		WaitForSingleObject(report_thread_event_, INFINITE);
		
	}
	is_active_ = false;
	
	DriverLog("DpHmdDriver::Cleanup()\n");
}
 

void DpHmdDriver::RunFrame()
{

	/*if (vr_device_index_ != vr::k_unTrackedDeviceIndexInvalid)
	{
		server_driver_host_->TrackedDevicePoseUpdated(vr_device_index_, GetPose(), sizeof(DriverPose_t));
	}*/
}


EVRInitError DpHmdDriver::Activate(uint32_t unObjectId)
{
	
	
	loop_ = true;
	report_thread_event_= CreateEvent(NULL, FALSE, FALSE, NULL);
	check_thread_event_= CreateEvent(NULL, FALSE, FALSE, NULL);
	_beginthreadex(NULL, 0, &DpStateReporterThread, this, 0, 0);
	_beginthreadex(NULL, 0, &DpStateCheckThread, this, 0, 0);
	OnActive(true);
	DriverLog("DpHmdDriver::Activate : unObjectId=%d\n", unObjectId);
	vr_device_index_ = unObjectId;
	SetProperties();
	HmdMatrix_SetIdentity(&m_eyeToHeadLeft);
	HmdMatrix_SetIdentity(&m_eyeToHeadRight);
	is_active_ = true;
	return VRInitError_None;
}

void DpHmdDriver::Deactivate()
{
	DriverLog("DpHmdDriver::Deactivate : \n");
	vr_device_index_ = vr::k_unTrackedDeviceIndexInvalid;
}

void DpHmdDriver::EnterStandby()
{
	DriverLog("DpHmdDriver::EnterStandby\n");
	return;
}

void * DpHmdDriver::GetComponent(const char *pchComponentNameAndVersion)
{
#ifndef NO_DP
	DriverLog("DpHmdDriver::GetComponent : %s\n", pchComponentNameAndVersion);
	if (!_stricmp(pchComponentNameAndVersion, vr::IVRDisplayComponent_Version))
	{
		return (vr::IVRDisplayComponent*)this;
	}

#endif // !NO_DP
 
	// override this to add a component to a driver
	return NULL;
}

/** debug request from a client */
void DpHmdDriver::DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize)
{
	DriverLog("DpHmdDriver::DebugRequest : \n");
	if (unResponseBufferSize >= 1)
		pchResponseBuffer[0] = 0;
}

void DpHmdDriver::GetWindowBounds(int32_t *pnX, int32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight)
{
	DriverLog("DpHmdDriver::GetWindowBounds : %d, %d, %d, %d\n", window_x_, window_y_, window_width_, window_height_);
	*pnX = window_x_;
	*pnY = window_y_;
	
	*pnWidth = window_height_;
	*pnHeight = window_width_;
	
}

bool DpHmdDriver::IsDisplayOnDesktop()
{
	
	return false;
}

bool DpHmdDriver::IsDisplayRealDisplay()
{
	 
	return true;
}

void DpHmdDriver::GetRecommendedRenderTargetSize(uint32_t *pnWidth, uint32_t *pnHeight)
{
	 
	float fScale = 1.5;
	render_width_ = *pnHeight = (uint32_t)((float)window_height_ * fScale);
	render_height_ = *pnWidth = (uint32_t)((float)window_width_ / 2 * fScale);
	 
	DriverLog("DpHmdDriver::GetRecommendedRenderTargetSize: pnWidth = %d, pnHeight = %d, fScale = %f \n", *pnWidth, *pnHeight, fScale);
}

void DpHmdDriver::GetEyeOutputViewport(EVREye eEye, uint32_t *pnX, uint32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight)
{

	*pnX = 0;
	*pnWidth = window_height_;
	*pnHeight = window_width_ / 2;
	if (eEye == EVREye::Eye_Right)
	{
		*pnY = 0;
	}
	else
	{
		*pnY = window_width_ / 2;
	}
	int steamvrverson = GetSteamvrVersion();
	if (steamvrverson < STEAMVRVERSIONDOOR || gConfigReader.GetReversal() == 1)
	{
		*pnX = 0;
		*pnWidth = window_height_;
		*pnHeight = window_width_ / 2;
		if (eEye == EVREye::Eye_Left)
		{
			*pnY = 0;
		}
		else
		{
			*pnY = window_width_ / 2;
		}
	}
	DriverLog("HmdDeviceDriver::GetEyeOutputViewport: eye=%d, x=%d, y=%d, width=%d, height=%d \n", eEye, *pnX, *pnY, *pnWidth, *pnHeight);

}
 
void DpHmdDriver::GetProjectionRaw(EVREye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom)
{
	/*const float aspect = window_width_ * 0.5 / window_height_;
#define DEGREES_TO_RADIANS 0.01745329251f

	float halfFovInRadians = 95* DEGREES_TO_RADIANS * 0.5f;
	float tangent = tan(halfFovInRadians);
	*pfLeft = -tangent;
	*pfRight = tangent;
	*pfTop = -tangent;
	*pfBottom = tangent;*/
	float size = 1.07;
	*pfLeft = -1 * size;
	*pfRight = 1 * size;
	*pfTop = -1 * size;
	*pfBottom = 1 * size;

	DriverLog("CFalconDeviceDriver::GetProjectionRaw: eye=%d, %f, %f, %f, %f, %f,%f\n", eEye, *pfLeft, *pfRight, *pfTop, *pfBottom, 101,1);
}

void DpHmdDriver::SetOffsetIpd_(float ipd_, float offset_ul, float offset_vl, float offset_ur, float offset_vr)
{
	 
	if (ipd_<=0.06)
	{
		offset_ipd_min_l_[0] = offset_ul;
		offset_ipd_min_l_[1] = offset_vl;
		offset_ipd_min_r_[0] = offset_ur;
		offset_ipd_min_r_[1] = offset_vr;
	} 
	else if(0.06 < ipd_ && ipd_ < 0.065)
	{
		offset_ipd_mid_l_[0] = offset_ul;
		offset_ipd_mid_l_[1] = offset_vl;
		offset_ipd_mid_r_[0] = offset_ur;
		offset_ipd_mid_r_[1] = offset_vr;
	}
	else if (ipd_ >0.065)
	{
		offset_ipd_max_l_[0] = offset_ul;
		offset_ipd_max_l_[1] = offset_vl;
		offset_ipd_max_r_[0] = offset_ur;
		offset_ipd_max_r_[1] = offset_vr;
	}

}
void DpHmdDriver::GetOffsetIpd_(float ipd_, float& offset_ul, float& offset_vl, float& offset_ur, float& offset_vr)
{
	if (ipd_ <= 0.06)
	{
		offset_ul=offset_ipd_min_l_[0] ;
		offset_vl=offset_ipd_min_l_[1] ;
		offset_ur = offset_ipd_min_r_[0];
		offset_vr = offset_ipd_min_r_[1];
	}
	else if (0.06 < ipd_ && ipd_ < 0.065)
	{
		offset_ul=offset_ipd_mid_l_[0] ;
		offset_vl= offset_ipd_mid_l_[1];
		offset_ur = offset_ipd_mid_r_[0];
		offset_vr = offset_ipd_mid_r_[1];
	}
	else if (ipd_ > 0.065)
	{
		offset_ul = offset_ipd_max_l_[0];
		offset_vl = offset_ipd_max_l_[1];
		offset_ur = offset_ipd_max_r_[0];
		offset_vr = offset_ipd_max_r_[1];
	}
	if (gLog)
	{
		DriverLog("get offset ipd=%f,offset0=%f,offset1=%f,offset2=%f,offset3=%f,offset4=%f,offset5=%f,offset6=%f, \
		offset7=%f, offset8=%f, offset9=%f, offset10=%f, offset11=%f", ipd_,
			offset_ipd_min_l_[0], offset_ipd_min_l_[1], offset_ipd_min_r_[0], offset_ipd_min_r_[1],
			offset_ipd_mid_l_[0], offset_ipd_mid_l_[1], offset_ipd_mid_r_[0], offset_ipd_mid_r_[1],
			offset_ipd_max_l_[0], offset_ipd_max_l_[1], offset_ipd_max_r_[0], offset_ipd_max_r_[1]);
	}
	
}

void DpHmdDriver::HmdMatrix_SetIdentity(vr::HmdMatrix34_t* pMatrix)
{
	pMatrix->m[0][0] = 1.f;
	pMatrix->m[0][1] = 0.f;
	pMatrix->m[0][2] = 0.f;
	pMatrix->m[0][3] = 0.f;
	pMatrix->m[1][0] = 0.f;
	pMatrix->m[1][1] = 1.f;
	pMatrix->m[1][2] = 0.f;
	pMatrix->m[1][3] = 0.f;
	pMatrix->m[2][0] = 0.f;
	pMatrix->m[2][1] = 0.f;
	pMatrix->m[2][2] = 1.f;
	pMatrix->m[2][3] = 0.f;
}
//DistortionCoordinates_t DpHmdDriver::ComputeDistortion(EVREye eEye, float fU, float fV)
//{
//
//	DistortionCoordinates_t coords;
//	float inuv[2] = { 0.0f, 0.0f };
//	float red[2], green[2], blue[2] = { 0.0f, 0.0f };
//
//	float tmp = fU;
//	fU = fV;
//	fV = tmp;
//
//	const float aspect = window_width_ * 0.5 / window_height_;
//	inuv[0] = fU;
//	inuv[1] = fV;
//	float offsetLeftMeters[2] = { -0.0f, 0.0f };
//	float offsetRightMeters[2] = { -0.0f,0.0f };
//
//	GetOffsetIpd_(ipd_, offsetLeftMeters[0], offsetLeftMeters[1], offsetRightMeters[0], offsetRightMeters[1]);
//
//
//	if (eEye == EVREye::Eye_Left)
//	{
//		//float fU_offset = fU - offsetLeftMeters[0] * 2 / 120.3624;
//		//float fV_offset = fV - offsetLeftMeters[1] / 63.07 ;
//		//Uv,fV转ndc坐标
//		inuv[0] = -1.0 + 2.0 * fU;
//		inuv[1] = 1.0 - 2.0 * fV;
//
//		WarpTexCoordChromaModeNdc(inuv, red, green, blue);
//		//printf(" 0 red0 %f red1 %f green0 %f green1%f blue0%f blue1 %f\n", red[0], red[1], green[0], green[1], blue[0], blue[1]);
//
//		//ndc坐标转Uv,fV
//		coords.rfRed[1] = (red[0] + 1) / 2.0 - offsetLeftMeters[0] * 2.0 / 120.3624;
//		coords.rfGreen[1] = (green[0] + 1) / 2.0 - offsetLeftMeters[0] * 2.0 / 120.3624;
//		coords.rfBlue[1] = (blue[0] + 1) / 2.0 - offsetLeftMeters[0] * 2.0 / 120.3624;
//		coords.rfRed[0] = (red[1] + 1) / 2.0 - offsetLeftMeters[1] / 63.07;
//		coords.rfGreen[0] = (green[1] + 1) / 2.0 - offsetLeftMeters[1] / 63.07;
//		coords.rfBlue[0] = (blue[1] + 1) / 2.0 - offsetLeftMeters[1] / 63.07;
//
//	}
//	else
//	{
//		//Uv,fV转ndc坐标
//		inuv[0] = -1.0 + 2.0 * fU;
//		inuv[1] = 1.0 - 2.0 * fV;
//
//		WarpTexCoordChromaModeNdc(inuv, red, green, blue);
//		printf(" 0 red0 %f red1 %f green0 %f green1%f blue0%f blue1 %f\n", red[0], red[1], green[0], green[1], blue[0], blue[1]);
//
//		//ndc坐标转Uv,fV
//		coords.rfRed[1] = (red[0] + 1) / 2.0 - offsetRightMeters[0] * 2.0 / 120.3624;
//		coords.rfGreen[1] = (green[0] + 1) / 2.0 - offsetRightMeters[0] * 2.0 / 120.3624;
//		coords.rfBlue[1] = (blue[0] + 1) / 2.0 - offsetRightMeters[0] * 2.0 / 120.3624;
//		coords.rfRed[0] = (red[1] + 1) / 2.0 - offsetRightMeters[1] / 63.07;
//		coords.rfGreen[0] = (green[1] + 1) / 2.0 - offsetRightMeters[1] / 63.07;
//		coords.rfBlue[0] = (blue[1] + 1) / 2.0 - offsetRightMeters[1] / 63.07;
//	}
//
//
//	//DriverLog("HmdDeviceDriver::ComputeDistortion: red0=%f,red1=%f,green0=%f,green1=%f,blue0=%f,blue1=%f\n", coords.rfRed[0], coords.rfRed[1], coords.rfGreen[0], coords.rfGreen[1], coords.rfBlue[0], coords.rfBlue[1]);
//	return coords;
//}
DistortionCoordinates_t DpHmdDriver::ComputeDistortion(EVREye eEye, float fU, float fV)
{
	//瞳距调节 三档 m_ipd_ = 58，63 or 69
	float f_ipd_move = 0;
	if (ipd_ <= 0.06)
	{
		f_ipd_move = 5 / 120.3624;
	}
	else if (ipd_ > 0.065)
	{
		f_ipd_move = -6 / 120.3624;
	}

	DistortionCoordinates_t coords;
	float inuv[2] = { 0.0f, 0.0f };
	float red[2], green[2], blue[2] = { 0.0f, 0.0f };

	float tmp = fU;
	fU = 1 - fV;
	fV = 1 - tmp;
	float resize_value = 1.04;
	float gain = 0.969858;
	float h_size = 1.0480349344978166; //1920/1832
	//float fov = 1.09131;
	float fov = 1.0;
	float size = 1.07;
	float size2 = 1.0;

	const float aspect = window_width_ * 0.5 / window_height_;

	fU = (fU - 0.5) * aspect * size2 + 0.5;
	inuv[0] = fU;
	fV = (fV - 0.5) * size2 + 0.5;
	inuv[1] = fV;
	float offsetLeftMeters[2] = { -0.0f, 0.0f };
	float offsetRightMeters[2] = { -0.0f, 0.0f };

	GetOffsetIpd_(ipd_, offsetLeftMeters[0], offsetLeftMeters[1], offsetRightMeters[0], offsetRightMeters[1]);


	if (eEye == EVREye::Eye_Left)
	{
		float fU_offset = offsetLeftMeters[0] * 2.0 / (WIDTH_METERS * 1000);
		float fV_offset = offsetLeftMeters[1] / (HEIGHT_METERS * 1000);
		//Uv,fV转ndc坐标
		inuv[0] = -1.0 + 2.0 * (fU + (fU_offset - f_ipd_move) * gain);
		inuv[0] = inuv[0] * fov;
		inuv[1] = 1.0 - 2.0 * (fV - fV_offset * gain);
		inuv[1] = inuv[1] * fov;

		WarpTexCoordChromaModeNdc(inuv, red, green, blue);
		//printf(" 0 red0 %f red1 %f green0 %f green1%f blue0%f blue1 %f\n", red[0], red[1], green[0], green[1], blue[0], blue[1]);

		//ndc坐标转Uv,fV
		coords.rfRed[0] = (red[0] / size + 1) / 2.0;
		coords.rfGreen[0] = (green[0] / size + 1) / 2.0;
		coords.rfBlue[0] = (blue[0] / size + 1) / 2.0;
		coords.rfRed[1] = (red[1] / size + 1) / 2.0;
		coords.rfGreen[1] = (green[1] / size + 1) / 2.0;
		coords.rfBlue[1] = (blue[1] / size + 1) / 2.0;

	}
	else
	{
		float fU_offset = offsetRightMeters[0] * 2.0 / (WIDTH_METERS * 1000);
		float fV_offset = offsetRightMeters[1] / (HEIGHT_METERS * 1000);
		//Uv,fV转ndc坐标
		inuv[0] = -1.0 + 2.0 * (fU + (fU_offset + f_ipd_move) * gain);
		inuv[0] = inuv[0] * fov;
		inuv[1] = 1.0 - 2.0 * (fV - fV_offset * gain);
		inuv[1] = inuv[1] * fov;

		WarpTexCoordChromaModeNdc(inuv, red, green, blue);
		//printf(" 0 red0 %f red1 %f green0 %f green1%f blue0%f blue1 %f\n", red[0], red[1], green[0], green[1], blue[0], blue[1]);

		//ndc坐标转Uv,fV
		coords.rfRed[0] = (red[0] / size + 1) / 2.0;
		coords.rfGreen[0] = (green[0] / size + 1) / 2.0;
		coords.rfBlue[0] = (blue[0] / size + 1) / 2.0;
		coords.rfRed[1] = (red[1] / size + 1) / 2.0;
		coords.rfGreen[1] = (green[1] / size + 1) / 2.0;
		coords.rfBlue[1] = (blue[1] / size + 1) / 2.0;
	}


	//DriverLog("HmdDeviceDriver::ComputeDistortion: red0=%f,red1=%f,green0=%f,green1=%f,blue0=%f,blue1=%f\n", coords.rfRed[0], coords.rfRed[1], coords.rfGreen[0], coords.rfGreen[1], coords.rfBlue[0], coords.rfBlue[1]);
	return coords;
}


DriverPose_t DpHmdDriver::GetPose()
{
	
	return driver_pose_;
}

void DpHmdDriver::SetProperties()
{
	PropertyContainerHandle_t ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(vr_device_index_);
	vr::EVRSettingsError eError;
	vr::VRDriverInput()->CreateBooleanComponent(ulPropertyContainer, "/proximity", &mHandelProximity);
	// Bool
	bool bValue   = true;
	vr::VRProperties()->SetBoolProperty(ulPropertyContainer, Prop_IsOnDesktop_Bool, false);
	vr::VRProperties()->SetBoolProperty(ulPropertyContainer, Prop_ContainsProximitySensor_Bool, false);
 
	bValue = false;
	vr::VRProperties()->SetBoolProperty(ulPropertyContainer, Prop_BlockServerShutdown_Bool, bValue);
 
	bValue = true;
	vr::VRProperties()->SetBoolProperty(ulPropertyContainer, Prop_WillDriftInYaw_Bool, bValue);
	vr::VRProperties()->SetBoolProperty(ulPropertyContainer, Prop_HasCamera_Bool, false);
 
	// Float
	
	vr::VRProperties()->SetFloatProperty(ulPropertyContainer, Prop_UserIpdMeters_Float, ipd_);
	vr::VRProperties()->SetFloatProperty(ulPropertyContainer, Prop_UserHeadToEyeDepthMeters_Float, 0.038);
	vr::VRProperties()->SetFloatProperty(ulPropertyContainer, Prop_DisplayFrequency_Float, display_frequency_);
	 
	
	seconds_from_vsync_to_photons_ = 0;
	vr::VRProperties()->SetFloatProperty(ulPropertyContainer, Prop_SecondsFromVsyncToPhotons_Float, seconds_from_vsync_to_photons_);
	// Int32
	vr::VRProperties()->SetInt32Property(ulPropertyContainer, Prop_DeviceClass_Int32, TrackedDeviceClass_HMD);
	vr::VRProperties()->SetInt32Property(ulPropertyContainer, Prop_EdidVendorID_Int32, edid_vendor_id_);
	
	vr::VRProperties()->SetUint64Property(ulPropertyContainer, Prop_CurrentUniverseId_Uint64, 12);
	// String
	vr::VRProperties()->SetStringProperty(ulPropertyContainer, Prop_ModelNumber_String, model_number_.c_str());
	vr::VRProperties()->SetStringProperty(ulPropertyContainer, Prop_SerialNumber_String,serial_number_.c_str());
	if (gConfigReader.GetControllerType() == 0)//pico
	{
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_ManufacturerName_String, "Pico");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_HardwareRevision_String, "V1.0");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_TrackingSystemName_String, "pico");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_ControllerType_String, "rift");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_InputProfilePath_String, "{oculus}/input/rift_profile.json");
		std::string register_type = "oculus/";
		register_type = register_type + serial_number_;
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, register_type.c_str());
	}
	else if (gConfigReader.GetControllerType()==3|| gConfigReader.GetControllerType() == 4)
	{
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, Prop_ModelNumber_String, model_number_.c_str());
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, Prop_SerialNumber_String, serial_number_.c_str());
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_ManufacturerName_String, "Pico");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_HardwareRevision_String, "V1.0");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_TrackingSystemName_String, "pico");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_ControllerType_String, "pico_hmd");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_InputProfilePath_String, "{pico}/input/pico_hmd_profile.json");
		std::string register_type = "piconeo3/";
		register_type = register_type + serial_number_;
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, register_type.c_str());
	}
	else if (gConfigReader.GetControllerType() == 1)//oculus
	{
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, "oculus/WMHD316Q512X13");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_ManufacturerName_String, "Oculus");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_HardwareRevision_String, "V1.0");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_TrackingSystemName_String, "oculus");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_ControllerType_String, "rift");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_InputProfilePath_String, "{oculus}/input/rift_profile.json");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, "oculus/WMHD316Q512X13");
	}
	else if (gConfigReader.GetControllerType() == 2)//htc
	{
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_ManufacturerName_String, "HTC");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_HardwareRevision_String, "V1.0");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_TrackingSystemName_String, "lighthouse");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_ControllerType_String, "vive_pro");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_InputProfilePath_String, "{htc}/input/vive_profile.json.json");
		vr::VRProperties()->SetStringProperty(ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, "htc/vive_proLHR-6FF22567");
	}
	vr::VRProperties()->SetStringProperty(ulPropertyContainer, Prop_NamedIconPathDeviceOff_String, "{pico}/icons/headset_off.png");
	vr::VRProperties()->SetStringProperty(ulPropertyContainer, Prop_NamedIconPathDeviceSearching_String, "{pico}/icons/headset_searching.png");
	vr::VRProperties()->SetStringProperty(ulPropertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, "{pico}/icons/headset_searching_alert.png");
	vr::VRProperties()->SetStringProperty(ulPropertyContainer, Prop_NamedIconPathDeviceReady_String, "{pico}/icons/headset_ready.png");
	vr::VRProperties()->SetStringProperty(ulPropertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, "{pico}/icons/headset_ready_alert.png");
	vr::VRProperties()->SetStringProperty(ulPropertyContainer, Prop_NamedIconPathDeviceNotReady_String, "{pico}/icons/headset_error.png");
	vr::VRProperties()->SetStringProperty(ulPropertyContainer, Prop_NamedIconPathDeviceStandby_String, "{pico}/icons/headset_standby.png");
	vr::VRProperties()->SetStringProperty(ulPropertyContainer, Prop_NamedIconPathDeviceAlertLow_String, "{pico}/icons/headset_ready_low.png");
	vr::VRProperties()->SetStringProperty(ulPropertyContainer, Prop_NamedIconPathDeviceStandbyAlert_String, "{pico}/icons/headset_ready_alert.png");
}

void DpHmdDriver::UpdatePose(RVR::RVRPoseHmdData* data)
{
	ReSetPose();
	 
	if (vr_device_index_ != vr::k_unTrackedDeviceIndexInvalid)
	{

		ExtractDriverPose(&driver_pose_, data);
		last_update_timestamp_ns_ = RVR::nowInNs();
		if (gLogOutHmdPose && gIsDebug)
		{
			char msg[2048] = { 0 };
			uint64_t timestamp = GetTimestampUs();
			sprintf_s(msg, "HmdData rotation %lf,%lf,%lf,%lf,  pose %lf,%lf,%lf, timestamp=%llu,hmd_time_stamp=%llu,sub_location=%llu,sub_remote=%llu\n",
				data->rotation.w, data->rotation.x, data->rotation.y, data->rotation.z,
				data->position.x, data->position.y, data->position.z,
				timestamp, data->poseTimeStamp,
				timestamp - timestamp_, data->poseTimeStamp - last_timestamp_);

			last_timestamp_ = data->poseTimeStamp;
			timestamp_ = timestamp;
			DriverLog(msg);
		}
		vr::VRDriverInput()->UpdateBooleanComponent(mHandelProximity, true, 0.0);
		//pxrConnectionRecorder::GetInstance()->UpdateConnectionStatus(data->valid);

		data->poseRecvTime = GetTimestampUs();
		vr::VRServerDriverHost()->TrackedDevicePoseUpdated(vr_device_index_, driver_pose_, sizeof(DriverPose_t));
		

	}
}
unsigned int __stdcall DpHmdDriver::DpStateReporterThread(LPVOID lpParameter)
{
	DpHmdDriver* dp_hmd = (DpHmdDriver*)lpParameter;
	dp_hmd->report_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in Local_addr;
	Local_addr.sin_family = AF_INET;
	Local_addr.sin_port = htons(29762);
	Local_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(dp_hmd->report_socket_, (struct sockaddr*)&Local_addr, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		DriverLog("绑定失败");
	}
	struct sockaddr_in client_addr;
	char buffer[1024] = { 0 };
	int len = sizeof(client_addr);
	while (dp_hmd->GetLoop_())
	{
		int ret=recvfrom(dp_hmd->report_socket_, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &len);
		if (ret==SOCKET_ERROR)
		{
			break;
		}
		if (strcmp(buffer,"dp_state_query")==0)
		{
			std::string report_msg = "dp_state:" + std::to_string(dp_hmd->GetDpRenderFlag_());
			int ret=sendto(dp_hmd->report_socket_, report_msg.c_str(), report_msg.length(), 0, (sockaddr*)&client_addr,sizeof(client_addr));
			//DriverLog("DpStateReporter reture %d",ret);
		}
		memset(buffer, 0, 1024);
	}
	SetEvent(dp_hmd->report_thread_event_);
	return 1;
}


unsigned int __stdcall DpHmdDriver::DpStateCheckThread(LPVOID lpParameter)
{
	   
	DpHmdDriver* dp_hmd = (DpHmdDriver*)lpParameter;
	int64_t last_ref = 0;
	float render_time = 0.f;
	Sleep(3000);
	 
	Compositor_FrameTiming *frame_timing=new Compositor_FrameTiming;
	while (dp_hmd->GetLoop_())
	{
		bool steamvr_is_exiting=vr::VRServerDriverHost()->IsExiting();
		if (dp_hmd->GetIsActived_()&& (!steamvr_is_exiting))
		{
			uint64_t timestamp = RVR::nowInNs();
			if ((timestamp- dp_hmd->last_update_timestamp_ns_)>2000000000)
			{
				dp_hmd->SetDpRenderFlag_(0);				 			 
				DriverLog("dp disconnected ");
			}
			else
			{
				dp_hmd->SetDpRenderFlag_(1);
			}
			 
			Sleep(800);
		}
	
	}
	SetEvent(dp_hmd->check_thread_event_);
	delete frame_timing;
	return 1;
}