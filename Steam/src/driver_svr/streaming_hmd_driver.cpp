//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------
#include "TcpSensorSocket.h"
#include <map>
#include <process.h>
#include "streaming_hmd_driver.h"
#include "driverlog.h"
#include "ConnectionRecorder.h"
#include "D3DHelper.h"
#include "Util.h"
#include "RVRLogger.h"
#include "RVRUtils.h"
#include "PluginManger.h"
#include "../RVRPlugin/IVEncPlugin.h"
#include "../RVRPlugin/RVRPluginDefinitions.h"
#include "TimeTool.h"
#include "filetool.h"
#include "runtime_wireless_mode_frame_depth_calc.h"
#include "driver_define.h"
#include "sensor_passer.h"
#include "config_reader.h"
#include"stringtool.h"
#ifndef NO_RTC
#include "ByteRtcMoudel.h"
#endif
#include "SensorManger.h"
#ifndef NO_USBBULK
#include "UsbBulkModule.h"
#endif



 
using namespace vr;
using namespace RVR;
using namespace pxr;

extern ConfigReader gConfigReader;
extern PluginManger gPluginManger;
#define MSG_VSYNC WM_USER+120
static  IVEncPlugin* mLeftEnvc = NULL;
static  IVEncPlugin* mRightEnvc = NULL;
extern int g_bit_rate_update;
extern bool setip;
extern bool g_long_sensor_;
extern std::string gDstip;
#include <D3DX11tex.h>
#include<timeapi.h>
#pragma comment(lib,"Winmm.lib")

uint32_t gBasePid = 0;
uint32_t gPid = 0;
int gControllerAcc = 0;
bool gIsDebug = false;
 
bool gLog = false;
bool gPictureTrace = false;
bool gLogOutControllerPose = false;
bool gLogOutHmdPose = false;
float gControllerPose = 0;
float gHmdTimeOffset = 0;
 
bool gPrintMsg=false;
bool g_savesubmit = false;
bool g_save_audio = false;
int idr_space = 0;
int pictureindex = 0;
bool g_test_sensor = false;
int g_test_sensor_mode = 0;
extern RVR::RVRPoseHmdData g_test_rvrhmd;
extern bool gDashboardActivated;
extern bool gOverlayShow;
unsigned int __stdcall StreamingHmdDriver::DepthCalcThread(LPVOID lpParameter) 
{
	StreamingHmdDriver* HmdObj = (StreamingHmdDriver*)lpParameter;
	MSG msg;
	PeekMessage(&msg, NULL, MSG_DEPTH, OUTTHREADMSG, PM_NOREMOVE);
 
	int picture_width = gConfigReader.GetEveWidth() - (gConfigReader.GetCutx() - gConfigReader.GetCutx() / gConfigReader.GetComPress()) * 2;
	int picture_height = gConfigReader.GetEveHeight() - (gConfigReader.GetCuty() - gConfigReader.GetCuty() / gConfigReader.GetComPress()) * 2;
	if (gConfigReader.GetLinearResolation() == 1)
	{
		picture_width = gConfigReader.GetEncoderWidth();
		picture_height = gConfigReader.GetEncoderHeight();
	}
	int calculate_picture_width =128;
	int calculate_picture_height = 128;
	int copy_dst_picture_width = gConfigReader.GetEveWidth()- gConfigReader.GetCutx()*2;// 1024  1312
	int copy_dst_picture_height = gConfigReader.GetEveHeight() - gConfigReader.GetCuty() * 2;
	const RuntimeWirelessModeFrameDepthCalc::DepthParam depth_param =
	{
		calculate_picture_width,
		calculate_picture_height,
		copy_dst_picture_width,
		copy_dst_picture_height,
		picture_width / 2 - copy_dst_picture_width/2,
		picture_height / 2 - copy_dst_picture_height/2
	};
	RuntimeWirelessModeFrameDepthCalc* depth_calc_ = new RuntimeWirelessModeFrameDepthCalc();

	depth_calc_->Startup(depth_param);
	float last_depth = -1;
	float base_div = 9.5f;
	if (HmdObj->m_nRenderWidth==1920)
	{
		base_div =12.8f;
	}
	else if (HmdObj->m_nRenderWidth == 1664)
	{
		base_div = 15.80f;
	}
	
	while (HmdObj->mLoop)
	{
		if (GetMessage(&msg, nullptr, MSG_DEPTH, OUTTHREADMSG)) //get msg from message queue
		{
			switch (msg.message)
			{
			case MSG_DEPTH: 
			{
				uint64_t start = RVR::nowInUs();
				HANDLE shared_handle = HmdObj->GetCompositor()->GetSharedTextureHandle(Eye_Left);
				depth_calc_->Submit(RuntimeWirelessModeFrameDepthCalc::StereoPictureType::kLeft, shared_handle);
				shared_handle = HmdObj->GetCompositor()->GetSharedTextureHandle(Eye_Right);
				depth_calc_->Submit(RuntimeWirelessModeFrameDepthCalc::StereoPictureType::kRight, shared_handle);
				depth_calc_->RenderToCalculateTex();
				int count = 0;
				float depth_value = depth_calc_->Compute2(count, base_div);
				if (depth_value > 0)
				{
					if (last_depth < 0)
					{
						last_depth = depth_value;
					}
					if (abs(last_depth - depth_value) > 8)
					{
						depth_value = last_depth * 0.4 + depth_value * 0.6;
					}
					last_depth = depth_value;
					HmdObj->SetDetph_(depth_value);
					gPluginManger.DoSetPoseDepth(HmdObj->GetDetph_());
					//RVR_LOG_A("depth=%f", depth_value);
					if ((0x8000 & GetAsyncKeyState('D')) != 0 && (0x8000 & GetAsyncKeyState('F')) != 0)
					{
						WCHAR strBuffer[256];
						swprintf_s(strBuffer, 256, L"depth = % f\n", depth_value);
						OutputDebugStringW(strBuffer);
					}


				}
				uint64_t end = RVR::nowInUs();
				if (gLog)
				{
					RVR_LOG_A("compute depth time=%d", (end - start) / 1000);
				}

				break;
			}
			case OUTTHREADMSG: 
			{
				break;
			}
			}
			
		}
	}
	depth_calc_->Shutdown();
	SetEvent(HmdObj->depthcalc_thread_event_);
	return 0;
}

unsigned int __stdcall StreamingHmdDriver::FunctionDebugThread(LPVOID lpParameter)
{

	StreamingHmdDriver* HmdObj = (StreamingHmdDriver*)lpParameter;
	MSG msg;
	
	PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
	int bufinedx = 0;
	while (HmdObj->mLoop)
	{

		if (((0x8000 & GetAsyncKeyState(VK_SHIFT)) != 0) && ((0x8000 & GetAsyncKeyState('1')) != 0) && ((0x8000 & GetAsyncKeyState('O')) != 0))
		{
			gIsDebug = true;
			RVR_LOG_A("driver debug open");
		}
		 

		if (((0x8000 & GetAsyncKeyState(VK_SHIFT)) != 0) && ((0x8000 & GetAsyncKeyState('0')) != 0))
		{
			gPictureTrace = false;
			gIsDebug = false;
			gPrintMsg = false;
			gLog = false;
			RVR_LOG_A("driver debug close");
		}
		if (gIsDebug)
		{
			if ((0x8000 & GetAsyncKeyState('X')) != 0 && (0x8000 & GetAsyncKeyState('O')) != 0)
			{
				g_test_sensor = true;
			}
			if ((0x8000 & GetAsyncKeyState('X')) != 0&& (0x8000 & GetAsyncKeyState('C'))!=0)
			{
				g_test_sensor = false;
			}
			if (g_test_sensor)
			{
				if ((0x8000 & GetAsyncKeyState('0')) != 0)
				{
					DriverLog("test sensor mode 0");
					g_test_sensor_mode = 0;
				}
				else if ((0x8000 & GetAsyncKeyState('1')) != 0)
				{
					DriverLog("test sensor mode 1");
					g_test_sensor_mode = 1;
				}
				else if ((0x8000 & GetAsyncKeyState('2')) != 0)
				{
					DriverLog("test sensor mode 2");
					g_test_sensor_mode = 2;
				}
				else if ((0x8000 & GetAsyncKeyState('3')) != 0)
				{
					DriverLog("test sensor mode 3");
					g_test_sensor_mode =3;
				}
				else if ((0x8000 & GetAsyncKeyState('4')) != 0)
				{
					DriverLog("test sensor mode 4");
					g_test_sensor_mode = 4;
				}
				else if ((0x8000 & GetAsyncKeyState('5')) != 0)
				{
					DriverLog("test sensor mode 5");
					g_test_sensor_mode = 5;
				}
				else if ((0x8000 & GetAsyncKeyState('6')) != 0)
				{
					DriverLog("test sensor mode 6");
					g_test_sensor_mode = 6;
				}
				else if ((0x8000 & GetAsyncKeyState('7')) != 0)
				{
					DriverLog("test sensor mode 7");
					g_test_sensor_mode = 7;
				}
				else if ((0x8000 & GetAsyncKeyState('8')) != 0)
				{
					DriverLog("test sensor mode 8");
					g_test_sensor_mode = 8;
				}
				else if ((0x8000 & GetAsyncKeyState('9')) != 0)
				{
					DriverLog("test sensor mode 9");
					g_test_sensor_mode = 9;
				}
			}
			if ((0x8000 & GetAsyncKeyState('S')) != 0 && (0x8000 & GetAsyncKeyState('A')) != 0)
			{
				g_save_audio = true;
				RVR_LOG_A("save_audio open");
			}
			if ((0x8000 & GetAsyncKeyState('A')) != 0 && (0x8000 & GetAsyncKeyState('C')) != 0)
			{
				g_save_audio = false;
				RVR_LOG_A("save_audio close");
			}
			if ((0x8000 & GetAsyncKeyState('L')) != 0 && (0x8000 & GetAsyncKeyState('O')) != 0)
			{

				gLog = true;
				RVR_LOG_A("gdriverLog open");
			}
			if ((0x8000 & GetAsyncKeyState('L')) != 0 && (0x8000 & GetAsyncKeyState('C')) != 0)
			{

				gLog = false;
				RVR_LOG_A("gLog close");
			}
			if ((0x8000 & GetAsyncKeyState('P')) != 0 && (0x8000 & GetAsyncKeyState('O')) != 0)
			{
				gPictureTrace=true;
			}
			if ((0x8000 & GetAsyncKeyState('P')) != 0 && (0x8000 & GetAsyncKeyState('C')) != 0)
			{
				gPictureTrace = false;
			}

			if ((0x8000 & GetAsyncKeyState('S')) != 0 && (0x8000 & GetAsyncKeyState('O')) != 0)
			{

				g_savesubmit = true;
				RVR_LOG_A("driver gLogOutControllerPose open");
			}
			if ((0x8000 & GetAsyncKeyState('S')) != 0 && (0x8000 & GetAsyncKeyState('O')) != 0)
			{

				g_savesubmit = true;
				RVR_LOG_A("driver gLogOutControllerPose open");
			}
			if ((0x8000 & GetAsyncKeyState('D')) != 0 && (0x8000 & GetAsyncKeyState('O')) != 0)
			{

				gLogOutControllerPose = true;
				RVR_LOG_A("driver gLogOutControllerPose open");
			}
			if ((0x8000 & GetAsyncKeyState('D')) != 0 && (0x8000 & GetAsyncKeyState('C')) != 0)
			{

				gLogOutControllerPose = false;
				RVR_LOG_A("gLogOutControllerPose close");
			}

			if ((0x8000 & GetAsyncKeyState('M')) != 0 && (0x8000 & GetAsyncKeyState('O')) != 0)
			{

				gPrintMsg = true;
				RVR_LOG_A("driver print msg open");
			}
			if ((0x8000 & GetAsyncKeyState('M')) != 0 && (0x8000 & GetAsyncKeyState('C')) != 0)
			{

				gPrintMsg = false;
				RVR_LOG_A("driver print msg close");
			}
			if ((0x8000 & GetAsyncKeyState('H')) != 0 && (0x8000 & GetAsyncKeyState('O')) != 0)
			{

				gLogOutHmdPose  = true;
				RVR_LOG_A("driver gLogOutHmdPose open");
			}
			if ((0x8000 & GetAsyncKeyState('H')) != 0 && (0x8000 & GetAsyncKeyState('C')) != 0)
			{

				gLogOutHmdPose = false;
				RVR_LOG_A("gLogOutHmdPose close");
			}
			if ((0x8000 & GetAsyncKeyState('R')) != 0 && (0x8000 & GetAsyncKeyState('S')) != 0)
			{
				vr::VRServerDriverHost()->SetRecommendedRenderTargetSize(HmdObj->GetVrDeviceId(), 2560, 2560);
			}
					 
		}
		Sleep(5);
		
		gConfigReader.GetMicWork_();
		gConfigReader.GetMicVolume_();
		gConfigReader.GetBright();
		gConfigReader.GetSaturation();
		gConfigReader.GetContrast();
		gConfigReader.GetAlpha();
		gConfigReader.GetSharper();
		gConfigReader.GetSharperWeight();
		gConfigReader.GetGamma();
		gConfigReader.GetLeftPitch();
		gConfigReader.GetLeftYaw();
		gConfigReader.GetLeftRoll();
		gConfigReader.GetLeftAddX();
		gConfigReader.GetLeftAddY();
		gConfigReader.GetLeftAddZ();

		gConfigReader.GetRightPitch();
		gConfigReader.GetRightYaw();
		gConfigReader.GetRightRoll();
		gConfigReader.GetRightAddX();
		gConfigReader.GetRightAddY();
		gConfigReader.GetRightAddZ();
		gControllerAcc = gConfigReader.GetControllerAccFlag();
		gConfigReader.GetDepthCompute();
		gConfigReader.GetSmoothController();
		gConfigReader.GetTcp();
		gConfigReader.GetNotEncode();
		gConfigReader.GetMaxSensorStore();
		gConfigReader.GetRtcOrBulkModeFromFile_();
		if (!FLT_EQUAL(gConfigReader.GetControllerpose(),gControllerPose))
		{
			gControllerPose = gConfigReader.GetControllerpose();
			RVR_LOG_A("controllerpose changed =%f", gControllerPose);
		}
		if (!FLT_EQUAL(gConfigReader.GetHmdpose(), gHmdTimeOffset))
		{
			gHmdTimeOffset = gConfigReader.GetHmdpose();
			RVR_LOG_A("hmdpose changed =%f", gHmdTimeOffset);
		}
		
	}
	SetEvent(HmdObj->function_debug_thread_event_);
	return 1;
}



 
//-----------------------------------------------------------------------------
StreamingHmdDriver::StreamingHmdDriver(RVRStub* mStubInstance, ID3D11Device* d3d11Device)
//-----------------------------------------------------------------------------
{
	mLoop = true;
	gLog = gConfigReader.GetLog() == 1 ? true : false;
	timeBeginPeriod(1);
	HANDLE ret = (HANDLE)_beginthreadex(NULL, 0, &FunctionDebugThread, this, 0, &mFunctionDebugThreadId);
	ret = (HANDLE)_beginthreadex(NULL, 0, &DepthCalcThread, this, 0, &mDepthCalcThreadId);
	
	this->d3d11device_ = d3d11Device;
	this->mStubInstance = mStubInstance;

	vr_device_index_ = vr::k_unTrackedDeviceIndexInvalid;
	m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;

	m_nRenderWidth = gConfigReader.GetEveWidth();
	m_nRenderHeight = gConfigReader.GetEveHeight();
#define ONE_SECOND_IN_NS 1000000000
	per_frame_ns_ = ONE_SECOND_IN_NS / gConfigReader.GetEncodeFps();
	string msg = " per_frame_ns_=" + std::to_string(per_frame_ns_);
	RVR_LOG_A(msg.c_str());
	m_sSerialNumber.assign("Pico Neo 2");
	m_sModelNumber.assign("Pico Neo 2");
	////modiefy dy dzhuang
	if (gConfigReader.GetHmdType().compare("neo2")==0)
	{
		 
		m_sSerialNumber.assign("Pico Neo 2");
		m_sModelNumber.assign("Pico Neo 2");
		 
		 
	} 
	else if(gConfigReader.GetHmdType().compare("neo3") == 0)
	{
		
		m_sSerialNumber.assign("Pico Neo 3");
		m_sModelNumber.assign("Pico Neo 3");	 
	}	
	else if (gConfigReader.GetHmdType().compare("phoenix") == 0)
	{

		m_sSerialNumber.assign("PICO 4");
		m_sModelNumber.assign("PICO 4");
	}
	if (gConfigReader.GetControllerType() == 1)
	{
		m_sSerialNumber.assign("WMHD316Q512X13");
		m_sModelNumber.assign("Oculus Rift CV1");
	}
	else if (gConfigReader.GetControllerType() == 2)
	{
		m_sSerialNumber.assign("LHR - 6FF22567");
		m_sModelNumber.assign("VIVE_Pro MV");
	}
	//end 
	startFrameTime = 0;

	DriverLog("driver_pico: Serial Number: %s\n", m_sSerialNumber.c_str());
	DriverLog("driver_pico: Model Number: %s\n", m_sModelNumber.c_str());
	DriverLog("driver_pico: Render Target: %d %d\n", m_nRenderWidth, m_nRenderHeight);

	textureSetLock = CreateMutex(nullptr, FALSE, L"TextureSetLock");
	poseHistoryLock = CreateMutex(nullptr, FALSE, L"PoseHistoryLock");

	depthcalc_thread_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	function_debug_thread_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	split_encode_thread_event_= CreateEvent(NULL, FALSE, FALSE, NULL);
	
	//mLeftConver = new RGBToNV12ConverterD3D11(d3d11Device, gpContext, gConfigReader.GetEveWidth(), gConfigReader.GetEveHeight());
	//mRightConver = new RGBToNV12ConverterD3D11(d3d11Device, gpContext, gConfigReader.GetEveWidth(), gConfigReader.GetEveHeight());

	poseDataLatest.valid = false;
	DriverLog("right encode init success");
	string device_id = GetMac();
	DriverLog(" go startup rtc device_id=%s", device_id.c_str());
	if (gConfigReader.GetAudioPicoModel() == 1)
	{
		audioCaptureSession = new AudioCaptureSession();
		DriverLog("audioCaptureSession");

	}
#ifndef NO_RTC
	if (gConfigReader.GetRtcOrBulkMode_()==1)
	{
		pico_streaming::ByteRtcMoudel::GetInstance()->SetAudioConfig(2, audioCaptureSession->GetSample(), 2, audioCaptureSession->GetSample());
		pico_streaming::ByteRtcMoudel::GetInstance()->SetVideoConfig(gConfigReader.GetEncoderWidth(), gConfigReader.GetEncoderHeight(), gConfigReader.GetEncodeFps()/2, gConfigReader.GetMaxBitRateValue() / 1000, gConfigReader.GetHEVC() == 1 ? 2 : 1);
		DriverLog("ByteRtcMoudel set video set audio");
		if (pico_streaming::ByteRtcMoudel::GetInstance()->Startup("127.0.0.1", (char*)device_id.c_str()) == false)
		{
			DriverLog("ByteRtcMoudel  start error");
		}
		DriverLog("ByteRtcMoudel start");
	}
#endif	
#ifndef NO_USBBULK

	//if (gConfigReader.GetRtcOrBulkMode_() == 2)
	//{
		pico_streaming::UsbBulkModule::GetInstance()->Start();
		//pico_streaming::UsbBulkModule::GetInstance(1)->Start();
		DriverLog("UsbBulkModule start");
	//}
#endif
	InitializeCriticalSection(&cs_pose_);
	InitializeCriticalSection(&cs_depth_);
	ReSetPose();
}

//-----------------------------------------------------------------------------
void StreamingHmdDriver::ClearTextureList()
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
StreamingHmdDriver::~StreamingHmdDriver()
//-----------------------------------------------------------------------------
{
	
	DeleteCriticalSection(&cs_pose_);
	DeleteCriticalSection(&cs_depth_);
	timeEndPeriod(1);
	 

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
	if (mLeftEnvc!=NULL)
	{
		mLeftEnvc->StopLoop();
	}
	if (mRightEnvc!=NULL)
	{
		mRightEnvc->StopLoop();
	}
	if (mLeftEnvc != NULL)
	{
		mLeftEnvc->Uninitialize();
	}
	if (mRightEnvc != NULL)
	{
		mRightEnvc->Uninitialize();
	}
	 
}
void StreamingHmdDriver::Cleanup() 
{
	SensorPasser::GetInstance()->SetNewSensorNotifyEvent();
	TcpSensorSocket::GetInstance()->CloseSocket();
	OnClearUp();
	mLoop = false;
	PostThreadMessage(mDepthCalcThreadId, OUTTHREADMSG, NULL, NULL);
#ifndef NO_RTC
	if (gConfigReader.GetRtcOrBulkMode_() == 1)
	{
		pico_streaming::ByteRtcMoudel::GetInstance()->Shutdown();
	}
#endif
#ifndef NO_USBBULK

	if (gConfigReader.GetRtcOrBulkMode_() ==2)
	{
		pico_streaming::UsbBulkModule::GetInstance()->Shutdown();
	}
#endif
	WaitForSingleObject(function_debug_thread_event_, INFINITE);
	WaitForSingleObject(depthcalc_thread_event_, INFINITE);
	if (gConfigReader.GetSplit_()==1&&is_actived_)
	{
		WaitForSingleObject(split_encode_thread_event_, INFINITE);
	}
	
}
void StreamingHmdDriver::SetIcons() 
{
	if (gConfigReader.GetHmdType().compare("neo2")==0)
	{
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceOff_String, "{pico}/icons_neo2/headset_off.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearching_String, "{pico}/icons_neo2/headset_searching.png");	
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, "{pico}/icons_neo2/headset_searching_alert.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReady_String,"{pico}/icons_neo2/headset_ready.png" );
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, "{pico}/icons_neo2/headset_ready_alert.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceNotReady_String, "{pico}/icons_neo2/headset_error.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandby_String, "{pico}/icons_neo2/headset_standby.png");		
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceAlertLow_String, "{pico}/icons_neo2/headset_ready_low.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandbyAlert_String, "{pico}/icons_neo2/headset_ready_alert.png");
	}
	else if (gConfigReader.GetHmdType().compare("neo3") == 0)
	{
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceOff_String, "{pico}/icons_neo3/headset_off.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearching_String, "{pico}/icons_neo3/headset_searching.gif");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, "{pico}/icons_neo3/headset_searching_alert.gif");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReady_String, "{pico}/icons_neo3/headset_ready.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, "{pico}/icons_neo3/headset_ready_alert.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceNotReady_String, "{pico}/icons_neo3/headset_error.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandby_String, "{pico}/icons_neo3/headset_standby.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceAlertLow_String, "{pico}/icons_neo3/headset_ready_low.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandbyAlert_String, "{pico}/icons_neo3/headset_ready_alert.png");
	}
	else if (gConfigReader.GetHmdType().compare("phoenix") == 0)
	{
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceOff_String, "{pico}/icons_phoenix/headset_off.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearching_String, "{pico}/icons_phoenix/headset_searching.gif");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, "{pico}/icons_phoenix/headset_searching_alert.gif");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReady_String, "{pico}/icons_phoenix/headset_ready.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, "{pico}/icons_phoenix/headset_ready_alert.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceNotReady_String, "{pico}/icons_phoenix/headset_error.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandby_String, "{pico}/icons_phoenix/headset_standby.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceAlertLow_String, "{pico}/icons_phoenix/headset_ready_low.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandbyAlert_String, "{pico}/icons_phoenix/headset_ready_alert.png");
	}
}
void StreamingHmdDriver::StartEncoder() 
{
	compositor.Initialize(d3d11device_, gConfigReader.GetEncoderWidth(), gConfigReader.GetEncoderHeight());
	RVR::VEncConfig encConfig = { 0 };
	encConfig.avgBitRate = gConfigReader.GetAverageBitRate();
	encConfig.maxBitRate = gConfigReader.GetMaxBitRateValue();
	encConfig.level = 52;
	encConfig.D3D11Device = d3d11device_;
	encConfig.fps = gConfigReader.GetEncodeFps();
	encConfig.width = gConfigReader.GetEncoderWidth();
    encConfig.height = gConfigReader.GetEncoderHeight();

	encConfig.minIQP = encConfig.minIQP = 0;
	encConfig.maxIQP = encConfig.maxPQP = 51;
	if (gConfigReader.BigPicture() == 1)
	{
		if (gConfigReader.GetSingleEncode() == 0)
		{
			encConfig.fps = encConfig.fps / 2;
		}
#ifdef FORCE_HALFFPS
		encConfig.fps = encConfig.fps / 2;
#endif
		
	}
	
	mLeftEnvc = gPluginManger.mGreateEnvcFunPtr();
	HRESULT retL = mLeftEnvc->Initialize(&encConfig);
	if (retL != S_OK)
	{
		left_encoder_init_ = false;
	}
	else
	{
		left_encoder_init_ = true;
	}


	if (gConfigReader.GetSingleEncode()==0)
	{
		mRightEnvc = gPluginManger.mGreateEnvcFunPtr();
		HRESULT retR = mRightEnvc->Initialize(&encConfig);
		if (retR != S_OK)
		{
			right_encoder_init_ = false;
		}
		else
		{
			right_encoder_init_ = true;
		}
	}
	else
	{
		right_encoder_init_ = true;
	}

	if (gConfigReader.GetSplit_()==1)
	{
	//	D3DHelper::CreateDevice(split_d3d11device_);
		split_to_encode_texture_=D3DHelper::CreateSharedTexture2D(d3d11device_, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, encConfig.width, encConfig.height, 1);
		_beginthreadex(NULL, 0, &split_encode_thread_, this, 0, 0);
	}
}
unsigned int  StreamingHmdDriver::split_encode_thread_(LPVOID lpParameter)
{
	StreamingHmdDriver* hmd_obj = (StreamingHmdDriver*)lpParameter;
	int frame_index = 0;
	
	int64_t encode_ts_space = 1000000000 / gConfigReader.GetEncodeFps();
	ID3D11Device *d3d11_device = hmd_obj->GetD3D11Device();
	ID3D11DeviceContext* context;
	d3d11_device->GetImmediateContext(&context);
	int64_t current_ts_u =0;
	int64_t current_ts_n =0;
	int64_t last_encode_ts_n = -1;
	while (hmd_obj->mLoop)
	{	
		current_ts_n = RVR::nowInNs();
		if ((current_ts_n - last_encode_ts_n) < encode_ts_space)
		{
			int sleep_t =( encode_ts_space-(current_ts_n - last_encode_ts_n)) / 1000000;
			if (sleep_t > 0)
			{
				Sleep(sleep_t);
				//RVR_LOG_A("Sleep split %d  encode mutex cout %f", sleep_t, (current_ts_n - last_encode_ts_n) / 1000000);
			}
			
		}
		int index = 0;
		ID3D11Texture2D* split_texture = nullptr;
		HANDLE split_handle = hmd_obj->GetCompositor()->GetSplitTextureHandleForEncoder(index);
		auto hr = d3d11_device->OpenSharedResource(const_cast<HANDLE>(split_handle)
			, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&split_texture));
	    if (FAILED(hr))
	    {
			RVR_LOG_A("open split texture error");
			Sleep(1);
			continue;
	    }
		int64_t mutb = RVR::nowInNs();
		auto* const mutex = D3D11ContextManager::AcquireMutex(split_texture);
		if (mutex==nullptr)
		{
			RVR_LOG_A("mutex null");
		}
		ID3D11Texture2D* copy_dst_texture = nullptr;
	
		if (context != nullptr)
		{
			
			copy_dst_texture = hmd_obj->GetSplitToEncodeTexture_();
			//copy_dst_texture = split_texture;
		    context->CopyResource(copy_dst_texture, split_texture);
			
			
		}
		D3D11ContextManager::ReleaseMutex(mutex);
		int64_t mute = RVR::nowInNs();
		//RVR_LOG_A("mutex cost=%f",(mute-mutb)/1000000.f);
		current_ts_u = RVR::nowInUs();
		hmd_obj->current_pose_mutex_.lock();
		RVR::RVRPoseHmdData encode_pose = hmd_obj->split_poses_[index];
		
		if (gPluginManger.mSetPoseCachePtr != NULL)
		{
			encode_pose.predictedTimeMs = hmd_obj->GetDetph_();
			if ((gConfigReader.GetRtcOrBulkMode_() == 1) || (gConfigReader.GetRtcOrBulkMode_() == 2))
			{
				SensorManger::GetInstance()->SaveSensor(encode_pose);
			}
			gPluginManger.DoSetPoseCache(&encode_pose);
		}
		hmd_obj->current_pose_mutex_.unlock();

		current_ts_u = RVR::nowInUs();
		int config_index = hmd_obj->GetEncodeFrameConfig(current_ts_u);
		current_ts_n = RVR::nowInNs();
		last_encode_ts_n = current_ts_n;
		if (frame_index % 2 == 0)
		{
			mLeftEnvc->QueueBuffer(copy_dst_texture, &hmd_obj->mFrameConfig[config_index]);
		}
		else
		{
			mRightEnvc->QueueBuffer(copy_dst_texture, &hmd_obj->mFrameConfig[config_index]);
		}
		mute = RVR::nowInNs();
		//RVR_LOG_A("mutex and encode and  cost=%f", (mute-current_ts_n) / 1000000.f);
		frame_index++;
		
	}
	SetEvent(hmd_obj->split_encode_thread_event_);
	return 1;
}
//-----------------------------------------------------------------------------
vr::EVRInitError StreamingHmdDriver::Activate(vr::TrackedDeviceIndex_t unObjectId)
//-----------------------------------------------------------------------------
{
	lastPresent=RVR::nowInNs();
	OnActive();
	StartEncoder();
	SetIpd_(gConfigReader.GetInterPupilDistance());

	vr_device_index_ = unObjectId;
	m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(vr_device_index_);
	

	vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_DriverDirectModeSendsVsyncEvents_Bool,true);
	
//	vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_DriverDirectModeSendsVsyncEvents_Bool, true); 
	vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_SecondsFromVsyncToPhotons_Float, 0.003 );
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ModelNumber_String, m_sModelNumber.c_str());
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RenderModelName_String, m_sModelNumber.c_str());
	vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_UserIpdMeters_Float, ipd_);
	vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_DisplayFrequency_Float, (float)(gConfigReader.GetFps()));
	vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_DoNotApplyPrediction_Bool, false);
	vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_ContainsProximitySensor_Bool, true);
	//modeify by dzhuang  2.3.5 版本里保留信息最多（加速，正方向校准。2.3.7有 效果较好的加速和 Oculus 映射原始写法和 出厂厂家等设置）
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RenderModelName_String, "generic_hmd");
	vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/proximity", &mHandelProximity);
	// return a constant that's not 0 (invalid) or 1 (reserved for Oculus)
	vr::VRProperties()->SetUint64Property(m_ulPropertyContainer, Prop_CurrentUniverseId_Uint64, 12);
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
		string register_type = "oculus/";
		register_type= register_type+ m_sSerialNumber;
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, register_type.c_str());
	}
	else if (gConfigReader.GetControllerType() == 1)//oculus
	{
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, "oculus/WMHD316Q512X13");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ManufacturerName_String, "Oculus");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_HardwareRevision_String, "V1.0");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_TrackingSystemName_String, "oculus");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "rift");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_InputProfilePath_String, "{oculus}/input/rift_profile.json");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, "oculus/WMHD316Q512X13");
	}
	else if (gConfigReader.GetControllerType() == 2)//htc
	{
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ManufacturerName_String, "HTC");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_HardwareRevision_String, "V1.0");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_TrackingSystemName_String, "lighthouse");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "vive_pro");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_InputProfilePath_String, "{htc}/input/vive_profile.json.json");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, "htc/vive_proLHR-5BC03915");
		 
	}
	else if (gConfigReader.GetControllerType() == 3|| gConfigReader.GetControllerType() == 4)//really pico
	{
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ManufacturerName_String, "Pico");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_HardwareRevision_String, "V1.0");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_TrackingSystemName_String, "pico");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "pico_hmd");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_InputProfilePath_String, "{pico}/input/pico_hmd_profile.json");
		string register_type = "piconeo3/";
		register_type = register_type + m_sSerialNumber;
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RegisteredDeviceType_String, register_type.c_str());
	}

	SetIcons();
	//end

	// return a constant that's not 0 (invalid) or 1 (reserved for Oculus)
	//vr::VRProperties()->SetUint64Property( m_ulPropertyContainer, Prop_CurrentUniverseId_Uint64, 2 );

	// avoid "not fullscreen" warnings from vrmonitor
	vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_IsOnDesktop_Bool, false);

	if (audioCaptureSession)
	{
		audioCaptureSession->Start();
	}
	is_actived_ = true;
	DriverLog("hmd actived");
	return VRInitError_None;
}

//-----------------------------------------------------------------------------
void StreamingHmdDriver::Deactivate()
//-----------------------------------------------------------------------------
{
	vr_device_index_ = vr::k_unTrackedDeviceIndexInvalid;

	if (audioCaptureSession)
	{
		audioCaptureSession->Stop();
	}
}

//-----------------------------------------------------------------------------
void StreamingHmdDriver::EnterStandby()
//-----------------------------------------------------------------------------
{
}

//-----------------------------------------------------------------------------
void* StreamingHmdDriver::GetComponent(const char* pchComponentNameAndVersion)
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
void StreamingHmdDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
//-----------------------------------------------------------------------------
{
	if (unResponseBufferSize >= 1)
		pchResponseBuffer[0] = 0;
}

//-----------------------------------------------------------------------------
void StreamingHmdDriver::GetWindowBounds(int32_t* pnX, int32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
//-----------------------------------------------------------------------------
{
	*pnX = 0;
	*pnY = 0;
	*pnWidth = m_nRenderWidth * 2;
	*pnHeight = m_nRenderHeight;
}

//-----------------------------------------------------------------------------
bool StreamingHmdDriver::IsDisplayOnDesktop()
//-----------------------------------------------------------------------------
{
	return false;
}

//-----------------------------------------------------------------------------
bool StreamingHmdDriver::IsDisplayRealDisplay()
//-----------------------------------------------------------------------------
{
	return false;
}

//-----------------------------------------------------------------------------
void StreamingHmdDriver::GetRecommendedRenderTargetSize(uint32_t* pnWidth, uint32_t* pnHeight)
//-----------------------------------------------------------------------------
{
	*pnWidth = m_nRenderWidth;
	*pnHeight = m_nRenderHeight;
}

//-----------------------------------------------------------------------------
void StreamingHmdDriver::GetEyeOutputViewport(EVREye eEye, uint32_t* pnX, uint32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
//-----------------------------------------------------------------------------
{
	*pnY = 0;
	*pnWidth = m_nRenderWidth;
	*pnHeight = m_nRenderHeight;
	*pnX = ((eEye == Eye_Left) ? 0 : m_nRenderWidth);
}

//-----------------------------------------------------------------------------
void StreamingHmdDriver::GetProjectionRaw(EVREye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom)
//-----------------------------------------------------------------------------
{


	float halfFovInRadians = gConfigReader.GetFov()* DEGREES_TO_RADIANS * 0.5f;
	float tangent = tan(halfFovInRadians);

	*pfLeft = -tangent;
	*pfRight = tangent;
	*pfTop = -tangent;
	*pfBottom = tangent;

}

//-----------------------------------------------------------------------------
DistortionCoordinates_t StreamingHmdDriver::ComputeDistortion(EVREye eEye, float fU, float fV)
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
void StreamingHmdDriver::ReSetPose()
{
	driver_pose_mutex_.lock();
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
	driver_pose_mutex_.unlock();
}
//-----------------------------------------------------------------------------
vr::DriverPose_t StreamingHmdDriver::GetPose()
//-----------------------------------------------------------------------------
{
	vr::DriverPose_t ret = {0};
	driver_pose_mutex_.lock();
	ret = driver_pose_;
	driver_pose_mutex_.unlock();
	return ret;
}

//-----------------------------------------------------------------------------
void StreamingHmdDriver::CacheLatestPose(RVR::RVRPoseHmdData* poseData)
//-----------------------------------------------------------------------------
{
	WaitForSingleObject(poseHistoryLock, INFINITE);
	poseDataLatest = *poseData;
	ReleaseMutex(poseHistoryLock);
}
void StreamingHmdDriver::SetInputRenderPose(RVR::RVRPoseHmdData hmd_pose)
{
	 
	input_render_pose_ = hmd_pose;
	input_render_pose_.hmdTimeStamp = hmd_pose.poseTimeStamp;

	 
}
void StreamingHmdDriver::UpdatePose(RVR::RVRPoseHmdData* data)
{
	if (data->poseTimeStamp<=0)
	{
		DriverLog("trace");
	}
	SetInputRenderPose(*data);
	SetOutputRenderPose(*data);
	
	if (gConfigReader.GetRtcOrBulkMode_() != 2)
	{
		ReSetPose();
	}
	
	if (setip == true)
	{
		gPluginManger.SetDstIp(gDstip);
		setip = false;
	}

	if (vr_device_index_ != vr::k_unTrackedDeviceIndexInvalid)
	{
		ExtractDriverPose(data);
		 
		if (gLogOutHmdPose && gIsDebug) 
		{
			char msg[2048] = { 0 };
			uint64_t timestamp = GetTimestampUs();
			/*sprintf_s(msg, "HmdData qw=%lf,qx=%lf,qy=%lf,qz=%lf,  PX=%lf,PY=%lf,PZ=%lf, timestamp=%ld,hmd_time_stamp=%ld\n",
				driver_pose_.qRotation.w, driver_pose_.qRotation.x, driver_pose_.qRotation.y, driver_pose_.qRotation.z,
				driver_pose_.vecPosition[0], driver_pose_.vecPosition[1], driver_pose_.vecPosition[2],
				timestamp, data->poseTimeStamp);*/
			sprintf_s(msg, "HmdData rotation %lf,%lf,%lf,%lf,  pose %lf,%lf,%lf, timestamp=%llu,hmd_time_stamp=%llu,sub_location=%llu,sub_remote=%llu\n",
				data->rotation.w, data->rotation.x, data->rotation.y, data->rotation.z,
				data->position.x, data->position.y, data->position.z,
				timestamp, data->poseTimeStamp,
				timestamp-timestamp_, data->poseTimeStamp- last_timestamp_);

			last_timestamp_ = data->poseTimeStamp;
			timestamp_ = timestamp;
			RVR_LOG_A(msg);
		}

		vr::VRDriverInput()->UpdateBooleanComponent(mHandelProximity, true, 0.0);
		//pxrConnectionRecorder::GetInstance()->UpdateConnectionStatus(data->valid);
		 
		//data->poseRecvTime = GetTimestampUs();
		 
		if (gControllerAcc == 0 || gConfigReader.GetAppRunValue() == 2 || gOverlayShow || gDashboardActivated || gConfigReader.GetAppRunValue() == 0)
		{
			driver_pose_.vecAcceleration[0] = driver_pose_.vecAcceleration[1] = driver_pose_.vecAcceleration[2] = 0;
			driver_pose_.vecVelocity[0] = driver_pose_.vecVelocity[1] = driver_pose_.vecVelocity[2] = 0;
			driver_pose_.vecAngularVelocity[0] = driver_pose_.vecAngularVelocity[1] = driver_pose_.vecAngularVelocity[2] = 0;
			driver_pose_.vecAngularAcceleration[0] = driver_pose_.vecAngularAcceleration[1] = driver_pose_.vecAngularAcceleration[2] = 0;
		}
		
		vr::VRServerDriverHost()->TrackedDevicePoseUpdated(vr_device_index_, driver_pose_, sizeof(DriverPose_t));
		if (data->valid)
		{
			RVR::RVRPoseHmdData datapose;
			memmove(&datapose, data, sizeof(RVRPoseHmdData));
			Operatie_Additional(&datapose);
		}
		
	}
}

//-----------------------------------------------------------------------------
void StreamingHmdDriver::ExtractDriverPose( RVR::RVRPoseHmdData* data)
//-----------------------------------------------------------------------------
{
	driver_pose_mutex_.lock();
	driver_pose_.poseIsValid = true;
	driver_pose_.result = TrackingResult_Running_OK;
	driver_pose_.deviceIsConnected = data->valid;

	driver_pose_.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
	driver_pose_.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);

	////Position
	driver_pose_.vecPosition[0] = data->position.x;
	driver_pose_.vecPosition[1] = data->position.y;
	driver_pose_.vecPosition[2] = data->position.z;

	//Orientation
	driver_pose_.qRotation.w = data->rotation.w;
	driver_pose_.qRotation.x = data->rotation.x;
	driver_pose_.qRotation.y = data->rotation.y;
	driver_pose_.qRotation.z = data->rotation.z;
	/*float r = sqrt(data->rotation.w*data->rotation.w + data->rotation.x*data->rotation.x +
		pose->qRotation.y* pose->qRotation.y + pose->qRotation.z*pose->qRotation.z);
	if (r < 0.0001)
		r = 0.0001;
*/
	driver_pose_.vecVelocity[0] = data->linearVelocity.x;
	driver_pose_.vecVelocity[1] = data->linearVelocity.y;
	driver_pose_.vecVelocity[2] = data->linearVelocity.z;

	driver_pose_.vecAcceleration[0] = data->linearAcceleration.x;
	driver_pose_.vecAcceleration[1] = data->linearAcceleration.y;
	driver_pose_.vecAcceleration[2] = data->linearAcceleration.z;

	driver_pose_.vecAngularVelocity[0] = data->angularVelocity.x;
	driver_pose_.vecAngularVelocity[1] = data->angularVelocity.y;
	driver_pose_.vecAngularVelocity[2] = data->angularVelocity.z;

	driver_pose_.vecAngularAcceleration[0] = 0;
	driver_pose_.vecAngularAcceleration[1] = 0;
	driver_pose_.vecAngularAcceleration[2] = 0;

	driver_pose_.poseTimeOffset= 0;
	driver_pose_.shouldApplyHeadModel = false;
	driver_pose_.willDriftInYaw = false;
	driver_pose_mutex_.unlock();
}
//void StreamingHmdDriver::UpdatePose(RVR::RVRPoseHmdData* data)
//{
//	if (gConfigReader.GetRtcOrBulkMode_() != 2)
//	{
//		ReSetPose();
//	}
//	
//	if (setip == true )
//	{
//		gPluginManger.SetDstIp(gDstip);
//		setip = false;
//	}
//	if (vr_device_index_ != vr::k_unTrackedDeviceIndexInvalid)
//	{
//		if (!data) {
//			data = mStubInstance->GetPose();
//		}
//		ExtractDriverPose(data);
//
//
//		if (gLogOutHmdPose && gIsDebug)
//		{
//			char msg[2048] = { 0 };
//			sprintf_s(msg, "HmdData qw=%lf,qx=%lf,qy=%lf,qz=%lf,  PX=%lf,PY=%lf,PZ=%lf\n",
//				driver_pose_.qRotation.w, driver_pose_.qRotation.x, driver_pose_.qRotation.y, driver_pose_.qRotation.z,
//				driver_pose_.vecPosition[0], driver_pose_.vecPosition[1], driver_pose_.vecPosition[2]);
//			RVR_LOG_A(msg);
//		}
//
//		vr::VRDriverInput()->UpdateBooleanComponent(mHandelProximity, true, 0.0);
//		//pxrConnectionRecorder::GetInstance()->UpdateConnectionStatus(data->valid);
//
//		data->poseRecvTime = GetTimestampUs();
//		vr::VRServerDriverHost()->TrackedDevicePoseUpdated(vr_device_index_, driver_pose_, sizeof(DriverPose_t));
//		if (data->valid)
//		{
//			Operatie_Additional(data);
//			AddPoseList(data);
//		}
//
//	}
//}
void StreamingHmdDriver::Operatie_Additional(RVR::RVRPoseHmdData* data) 
{
	if (data->predictedTimeMs > -999.00f)
	{
		//idr_immediately_ = false;
		//change_gop_ = false;
	}
	else if (data->predictedTimeMs < -999.00f && data->predictedTimeMs>-1001.00f)
	{
		idr_immediately_ = 0;
		//change_gop_ = true;
	}
	else if (data->predictedTimeMs < -1999.00f && data->predictedTimeMs>-2001.00f)
	{
		if (gConfigReader.BigPicture()==0)
		{
			idr_immediately_ = 1;
		} 
		else
		{
			idr_immediately_ = 2;
		}
		
		//change_gop_ = false;
	}
	else if (data->predictedTimeMs < -2999.00f && data->predictedTimeMs>-3001.00f)
	{
		if (gConfigReader.BigPicture() == 0)
		{
			idr_immediately_ = 1;
		}
		else
		{
			idr_immediately_ = 2;
		}
		change_gop_ = true;
	}
	else
	{
		//idr_immediately_ = false;
		//change_gop_ = false;
	}
	if (message_loop_time_%5==0)
	{
		if ((data->predictedTimeMs>0.05f)&&( data->predictedTimeMs<0.08))
		{
			float ipd_data = data->predictedTimeMs;
			if (!FLT_EQUAL(ipd_data, ipd_))
			{
				SetIpd_(ipd_data);
				PropertyContainerHandle_t ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(vr_device_index_);
			    //delete ipd pop box
				//vr::VRProperties()->SetFloatProperty(ulPropertyContainer, Prop_UserIpdMeters_Float,ipd_);
			}
		}
		
	}
	message_loop_time_++;
}
void StreamingHmdDriver::AddPoseList(RVR::RVRPoseHmdData* pose)
{
	EnterCriticalSection(&cs_pose_);
	pose_list_[pose_list_index_] = *pose;
	pose_list_index_ = (pose_list_index_ + 1) % LISTNUM;
	LeaveCriticalSection(&cs_pose_);
}
bool StreamingHmdDriver::FindAndDelete(RVR::RVRPoseHmdData pose_in, RVR::RVRPoseHmdData& retur_value_pose)
{
	retur_value_pose = pose_in;
	EnterCriticalSection(&cs_pose_);
	int i = pose_list_index_;
	bool find_flag = false;
	int loop_time = 0;
	while (i < LISTNUM)
	{
		RVR::RVRPoseHmdData pose = pose_list_[i % LISTNUM];
		if (FLT_EQUAL(pose.position.x, pose_in.position.x) && FLT_EQUAL(pose.position.y, pose_in.position.y) && FLT_EQUAL(pose.position.z, pose_in.position.z) &&
			FLT_EQUAL(pose.rotation.x, pose_in.rotation.x) && FLT_EQUAL(pose.rotation.y, pose_in.rotation.y) && FLT_EQUAL(pose.rotation.z, pose_in.rotation.z) && FLT_EQUAL(pose.rotation.w, pose_in.rotation.w))
		{
			find_flag = true;
			retur_value_pose = pose;
			break;
		}
		i = (i + 1) % LISTNUM;
		loop_time++;
		if (loop_time > LISTNUM)
		{
			break;
		}
	}


	//while (i < LISTNUM)
	//{
	//	RVR::RVRPoseHmdData pose = pose_list_[i % LISTNUM];
	//	if (FLT_EQUAL_2(pose.position.x, pose_in.position.x) && FLT_EQUAL_2(pose.position.y, pose_in.position.y) && FLT_EQUAL_2(pose.position.z, pose_in.position.z) &&
	//		FLT_EQUAL_2(pose.rotation.x, pose_in.rotation.x) && FLT_EQUAL_2(pose.rotation.y, pose_in.rotation.y) && FLT_EQUAL_2(pose.rotation.z, pose_in.rotation.z) && FLT_EQUAL_2(pose.rotation.w, pose_in.rotation.w))
	//	{
	//		find_flag = true;
	//		retur_value_pose = pose;
	//		break;
	//	}
	//	i = (i + 1) % LISTNUM;
	//	loop_time++;
	//	if (loop_time > LISTNUM)
	//	{
	//		break;
	//	}
	//}

	//while (i < LISTNUM)
	//{
	//	RVR::RVRPoseHmdData pose = pose_list_[i % LISTNUM];
	//	if (FLT_EQUAL_3(pose.position.x, pose_in.position.x) && FLT_EQUAL_3(pose.position.y, pose_in.position.y) && FLT_EQUAL_3(pose.position.z, pose_in.position.z) &&
	//		FLT_EQUAL_3(pose.rotation.x, pose_in.rotation.x) && FLT_EQUAL_3(pose.rotation.y, pose_in.rotation.y) && FLT_EQUAL_3(pose.rotation.z, pose_in.rotation.z) && FLT_EQUAL_3(pose.rotation.w, pose_in.rotation.w))
	//	{
	//		find_flag = true;
	//		retur_value_pose = pose;
	//		break;
	//	}
	//	i = (i + 1) % LISTNUM;
	//	loop_time++;
	//	if (loop_time > LISTNUM)
	//	{
	//		break;
	//	}
	//}
	LeaveCriticalSection(&cs_pose_);
	if (find_flag == false)
	{
		return find_flag;
	}
	return find_flag;
}
//-----------------------------------------------------------------------------
void StreamingHmdDriver::RunFrame()
//-----------------------------------------------------------------------------
{
//	RVR_LOG_A("run frame");
	// vr::VRServerDriverHost()->TrackedDevicePoseUpdated(vr_device_index_, driver_pose_, sizeof(DriverPose_t));
}

//-----------------------------------------------------------------------------
void StreamingHmdDriver::GetNextSwapTextureSetIndex(vr::SharedTextureHandle_t sharedTextureHandles[2], uint32_t(*pIndices)[2])
//-----------------------------------------------------------------------------
{
	{
		(*pIndices)[0]++;
		(*pIndices)[0] %= 3;
		(*pIndices)[1]++;
		(*pIndices)[1] %= 3;
	}

}

void StreamingHmdDriver::GetFrameTiming(vr::DriverDirectMode_FrameTiming* pFrameTiming)
{
	//RVR_LOG_A("GetFrameTiming  size= %d m_nNumFramePresents=%d  m_nNumMisPresented=%d m_nNumDroppedFrames=%d m_nReprojectionFlags=%d", 
		//pFrameTiming->m_nSize, pFrameTiming->m_nNumFramePresents, pFrameTiming->m_nNumMisPresented,pFrameTiming->m_nNumDroppedFrames ,pFrameTiming->m_nReprojectionFlags);
}

////-----------------------------------------------------------------------------
//void StreamingHmdDriver::CreateSwapTextureSet(uint32_t unPid, const SwapTextureSetDesc_t* pSwapTextureSetDesc, vr::SharedTextureHandle_t(*pSharedTextureHandles)[3])
////-----------------------------------------------------------------------------
//{
//	TextureSet* textureSet = new TextureSet();
//	for (int i = 0; i < 3; i++)
//	{
//
//		ID3D11Texture2D* texture = D3DHelper::CreateSharedTexture2D(d3d11Device, (DXGI_FORMAT)(pSwapTextureSetDesc->nFormat), pSwapTextureSetDesc->nWidth, pSwapTextureSetDesc->nHeight, pSwapTextureSetDesc->nSampleCount);
//		HANDLE sharedResourceHandle = D3DHelper::CreateSharedResource0(texture);
//		textureSet->texture[i] = texture;
//		textureSet->handle[i] = sharedResourceHandle;
//
//		//Output expected from CreateSwapTextureSet
//		(*pSharedTextureHandles)[i] = (vr::SharedTextureHandle_t)sharedResourceHandle;
//	}
//
//	textureSet->unPid = unPid;
//	WaitForSingleObject(textureSetLock, INFINITE);
//	textureSets.push_back(textureSet);
//	ReleaseMutex(textureSetLock);
//}
//
////-----------------------------------------------------------------------------
StreamingHmdDriver::TextureSet::~TextureSet()
//-----------------------------------------------------------------------------
{
	for (int i = 0; i < 3; i++)
	{
		D3DHelper::DeleteTexture2D(texture[i]);
		D3DHelper::DeleteSharedResource(handle[i]);
	}
}

//-----------------------------------------------------------------------------
void StreamingHmdDriver::DestroySwapTextureSet(vr::SharedTextureHandle_t sharedTextureHandle)
//-----------------------------------------------------------------------------
{
	WaitForSingleObject(textureSetLock, INFINITE);
	bool bFound = false;
	for (auto it = textureSets.begin(); it != textureSets.end();)
	{
		for (int i = 0; i < 3; i++)
		{
			if ((*it)->handle[i] == (HANDLE)sharedTextureHandle)
			{
				TextureSet* textureSet = *it;
				it = textureSets.erase(it);
				delete textureSet;
				bFound = true;
				break;
			}
		}

		if (bFound) break;
		else {
			++it;
		}
	}
	ReleaseMutex(textureSetLock);
}

//-----------------------------------------------------------------------------
void StreamingHmdDriver::DestroyAllSwapTextureSets(uint32_t unPid)
//-----------------------------------------------------------------------------
{
	WaitForSingleObject(textureSetLock, INFINITE);
	for (auto it = textureSets.begin(); it != textureSets.end();)
	{
		if ((*it)->unPid == unPid)
		{
			TextureSet* textureSet = *it;
			it = textureSets.erase(it);
			delete textureSet;
		}
		else {
			++it;
		}
	}
	ReleaseMutex(textureSetLock);
}

void StreamingHmdDriver::CreateSwapTextureSet(uint32_t unPid, const SwapTextureSetDesc_t* pSwapTextureSetDesc, SwapTextureSet_t* pOutSwapTextureSet)
{
	if (gBasePid ==0)
	{
		gBasePid = unPid;
	}
	gPid = unPid;
	DriverLog("gpid=%d pid=%d",gBasePid,gPid);
	if (pSwapTextureSetDesc->nHeight == 0 || pSwapTextureSetDesc->nWidth == 0) 
	{
		return;
	}
	TextureSet* textureSet = new TextureSet();
	for (int i = 0; i < 3; i++)
	{
	 
		ID3D11Texture2D* texture = D3DHelper::CreateSharedTexture2D(d3d11device_, (DXGI_FORMAT)(pSwapTextureSetDesc->nFormat), pSwapTextureSetDesc->nWidth, pSwapTextureSetDesc->nHeight, pSwapTextureSetDesc->nSampleCount);
		//ID3D11Texture2D* texture = D3DHelper::CreateSharedMipMapTexture2D(d3d11device_, (DXGI_FORMAT)(pSwapTextureSetDesc->nFormat), pSwapTextureSetDesc->nWidth, pSwapTextureSetDesc->nHeight, pSwapTextureSetDesc->nSampleCount);

		HANDLE sharedResourceHandle = NULL;
		if (texture!=NULL)
		{
			sharedResourceHandle = D3DHelper::CreateSharedResource0(texture);
		}
		textureSet->texture[i] = texture;
		textureSet->handle[i] = sharedResourceHandle;

		//Output expected from CreateSwapTextureSet
		//(*pSharedTextureHandles)[i] = (vr::SharedTextureHandle_t)sharedResourceHandle;
		pOutSwapTextureSet->rSharedTextureHandles[i] = (vr::SharedTextureHandle_t)sharedResourceHandle;
	}

	textureSet->unPid = unPid;
	WaitForSingleObject(textureSetLock, INFINITE);
	textureSets.push_back(textureSet);
	ReleaseMutex(textureSetLock);
	last_create_texture_ts = RVR::nowInNs();
}
//
int submitindex = 0;
void StreamingHmdDriver::SubmitLayer(const SubmitLayerPerEye_t(&perEye)[2])
{
	 
	if (compositor.NumberOfLayers() == 0)
	{
		picoRenderStart = RVR::nowInNs();
		out_render_pose_.endGameRenderStamp = picoRenderStart;
		//RVR_LOG_A("game render cost= %lld", out_render_pose_.endGameRenderStamp-out_render_pose_.beginGameRenderStamp);
		//Depth Texture from the first layer
		depthTexture[Eye::kLeft] = D3DHelper::AsTexture(d3d11device_, (HANDLE)(perEye[Eye::kLeft].hDepthTexture));
		depthTexture[Eye::kRight] = D3DHelper::AsTexture(d3d11device_, (HANDLE)(perEye[Eye::kRight].hDepthTexture));
	}
	if (gLog)
	{
		RVR_LOG_A("submitlayer  size= %d", compositor.NumberOfLayers());
	}
	{ // Add layer
		Compositor::LayerDesc layerDesc;

		layerDesc.left.texture = D3DHelper::AsTexture(d3d11device_, (HANDLE)(perEye[Eye::kLeft].hTexture));
		if (gPictureTrace)
		{
			if (compositor.NumberOfLayers() == 0)
			{
				wstring filename = L".//picturelayer//" + to_wstring(submitindex) + L".jpg";
				D3DHelper::SaveTextureToFile(d3d11device_, layerDesc.left.texture, filename.c_str());
				submitindex++;
			}
		}
		
		set_bounds(layerDesc.left.bounds, &(perEye[Eye::kLeft].bounds));

		layerDesc.right.texture = D3DHelper::AsTexture(d3d11device_, (HANDLE)(perEye[Eye::kRight].hTexture));
		set_bounds(layerDesc.right.bounds, &(perEye[Eye::kRight].bounds));

		compositor.Add(&layerDesc);
	}
	
	SetOutputRenderPoseFromMatrix34(&out_render_pose_, &perEye[Eye::kLeft].mHmdPose);

	char msg[2048] = { 0 };
	sprintf_s(msg, "get rotation %lf,%lf,%lf,%lf,  pose %lf,%lf,%lf  ts=%lld\n",
		out_render_pose_.rotation.w, out_render_pose_.rotation.x, out_render_pose_.rotation.y, out_render_pose_.rotation.z,
		-out_render_pose_.position.x, -out_render_pose_.position.y, -out_render_pose_.position.z,(RVR::nowInNs()- endPresent)/1000000);

	
	//RVR_LOG_A(msg);
	//if (g_test_sensor&&(g_test_sensor_mode>1))
	//{
	//	out_render_pose_.position = g_test_rvrhmd.position;
	//	out_render_pose_.rotation = g_test_rvrhmd.rotation;
	//}
	//RVR::RVRPoseHmdData out_pose;
	//if (gConfigReader.GetFindHistoryPose()==1)
	//{
	//	bool ret = FindAndDelete(out_render_pose_, out_pose);
	//	if (ret)
	//	{
	//		out_render_pose_.poseRecvTime = out_pose.poseRecvTime;
	//		//RVR_LOG_A("find ok");
	//	}
	//	else
	//	{
	//		//RVR_LOG_A("find  no");
	//	}
	//}
	
	////
	/*RVR::RVRPoseHmdData RightRenderPose;
	ExtractRVRPoseHmdData(&RightRenderPose, &perEye[Eye::kRight].mHmdPose);
	string msg;
	msg = "left p:" + to_string(out_render_pose_.position.x)+":"+to_string(out_render_pose_.position.y) + ":"
		+ to_string(out_render_pose_.position.z)+"r:"+ to_string(out_render_pose_.rotation.w) + ":" + to_string(out_render_pose_.rotation.x)
		+ ":" + to_string(out_render_pose_.rotation.y) + ":" + to_string(out_render_pose_.rotation.z);
	RVR_LOG_A(msg.c_str());

	msg = "right p:" + to_string(RightRenderPose.position.x) + ":" + to_string(RightRenderPose.position.y) + ":"
		+ to_string(RightRenderPose.position.z) + "r:" + to_string(RightRenderPose.rotation.w) + ":" + to_string(RightRenderPose.rotation.x)
		+ ":" + to_string(RightRenderPose.rotation.y) + ":" + to_string(RightRenderPose.rotation.z);
	RVR_LOG_A(msg.c_str());*/
}

//-----------------------------------------------------------------------------
//void StreamingHmdDriver::SubmitLayer(const SubmitLayerPerEye_t(&perEye)[2], const vr::HmdMatrix34_t* pPose)
////-----------------------------------------------------------------------------
//{
//	
//	if (compositor.NumberOfLayers() == 0)
//	{
//		//The Current Pose -- Required for passing this to the client along with the encoded frame.
//		ExtractRVRPoseHmdData(&out_render_pose_, pPose);
//
//		//Depth Texture from the first layer
//		depthTexture[Eye::kLeft] = D3DHelper::AsTexture(d3d11Device, (HANDLE)(perEye[Eye::kLeft].hDepthTexture));
//		depthTexture[Eye::kRight] = D3DHelper::AsTexture(d3d11Device, (HANDLE)(perEye[Eye::kRight].hDepthTexture));
//	}
//
//	{ // Add layer
//		Compositor::LayerDesc layerDesc;
//
//		layerDesc.left.texture = D3DHelper::AsTexture(d3d11Device, (HANDLE)(perEye[Eye::kLeft].hTexture));
//		
//		//D3DHelper::SaveTextureToFile(d3d11Device, layerDesc.left.texture, L"sdfsdf.jpg");
//		set_bounds(layerDesc.left.bounds, &(perEye[Eye::kLeft].bounds));
//
//		layerDesc.right.texture = D3DHelper::AsTexture(d3d11Device, (HANDLE)(perEye[Eye::kRight].hTexture));
//		set_bounds(layerDesc.right.bounds, &(perEye[Eye::kRight].bounds));
//
//		compositor.Add(&layerDesc);
//	}
//}

int64_t countPresent = 0;
int64_t firstPresent = -1;
uint64_t last_t = 0;

//-----------------------------------------------------------------------------
void StreamingHmdDriver::Present(vr::SharedTextureHandle_t syncTexture)
//-----------------------------------------------------------------------------
{

	IDXGIKeyedMutex* pKeyedMutex = D3DHelper::AcquireMutex(d3d11device_, (HANDLE)syncTexture);
	if (pKeyedMutex != nullptr)
	{
		if (gConfigReader.BigPicture() == 1)
		{
			float q1[4] = { 0 };
			float q2[4] = { 0 };
			RVR::RVRPoseHmdData current_pose = GetOutputRenderPose();
			int frame_index = frame_index_ % 2;
			if (gConfigReader.GetSingleEncode() == 1)
			{
				frame_index = 0;
			}

			compositor.Merge2to1(frame_index, hmd_data, left_data, right_data, current_pose.poseTimeStamp, current_pose.hmd_index);
		}
		else
		{
			compositor.Merge();
		}

	}
	D3DHelper::ReleaseMutex(pKeyedMutex);
	if (gLog)
	{
		picoRenderEnd = RVR::nowInNs();
		RVR_LOG_A("layers render time=%f", float(picoRenderEnd - picoRenderStart) / 1000000.0f);
	}

}
int StreamingHmdDriver::GetEncodeFrameConfig(int64_t CopyStart)
{
	RVR::VEncFrameConfig venc_config;
	venc_config = { 0 };
	venc_config.width = gConfigReader.GetEveWidth();
	venc_config.height = gConfigReader.GetEveHeight();
	venc_config.flags = 0;

	if (gConfigReader.GetInsertIdrEnable() == 1)
	{
		if (idr_immediately_ > 0)
		{
			if (idr_space > gConfigReader.GetEncodeFps() * 1)
			{
				venc_config.flags = venc_config.flags | RVR::VENC_FORCE_IDR;
				RVR_LOG_A("idr_immediately");
				idr_immediately_--;
				idr_space = 0;
			}
		}
	}

	idr_space++;
	if (change_gop_)
	{
		venc_config.flags = venc_config.flags | RVR::VENC_GOP_UPDATE;
		change_gop_ = false;
	}
	if (g_bit_rate_update == -1)
	{
		venc_config.flags = venc_config.flags | RVR::VENC_BITRATE_UPDATE;
		venc_config.avgBitRate = -1;
		g_bit_rate_update = 0;
	}
	picoRenderCostUs = CopyStart - picoRenderStart / 1000;
	venc_config.timestamp = picoRenderCostUs;
	int configindex = mFrameConfigIndex % 3;
	mFrameConfig[configindex] = venc_config;
#ifndef NO_RTC
	if (frame_index_%2==0)
	{
		if (gConfigReader.GetRtcOrBulkMode_() == 1)
		{
			int64_t timestamp = RVR::nowInNs();
			if (pico_streaming::ByteRtcMoudel::GetInstance()->left_idr_flag_)
			{
				mFrameConfig[configindex].flags = venc_config.flags | RVR::VENC_FORCE_IDR;
				pico_streaming::ByteRtcMoudel::GetInstance()->left_idr_flag_ = false;
			}
			if (pico_streaming::ByteRtcMoudel::GetInstance()->left_update_rate_flag_ && ((timestamp - hmd_active_timestamp_)) > 1000000000)
			{
				venc_config.flags = mFrameConfig[configindex].flags;
				mFrameConfig[configindex].flags = venc_config.flags | RVR::VENC_BITRATE_UPDATE;
				mFrameConfig[configindex].avgBitRate = pico_streaming::ByteRtcMoudel::GetInstance()->left_suggest_rate_;
				pico_streaming::ByteRtcMoudel::GetInstance()->left_update_rate_flag_ = false;
				pico_streaming::ByteRtcMoudel::GetInstance()->left_suggest_rate_ = 0;

			}
		}
	}
	else 
	{

		if (gConfigReader.GetRtcOrBulkMode_() == 1)
		{
			int64_t timestamp = RVR::nowInNs();
			if (pico_streaming::ByteRtcMoudel::GetInstance()->right_idr_flag_)
			{
				mFrameConfig[configindex].flags = venc_config.flags | RVR::VENC_FORCE_IDR;
				pico_streaming::ByteRtcMoudel::GetInstance()->right_idr_flag_ = false;
			}
			if (pico_streaming::ByteRtcMoudel::GetInstance()->right_update_rate_flag_ && ((timestamp - hmd_active_timestamp_)) > 1000000000)
			{
				venc_config.flags = mFrameConfig[configindex].flags;
				mFrameConfig[configindex].flags = venc_config.flags | RVR::VENC_BITRATE_UPDATE;
				mFrameConfig[configindex].avgBitRate = pico_streaming::ByteRtcMoudel::GetInstance()->right_suggest_rate_;
				pico_streaming::ByteRtcMoudel::GetInstance()->right_update_rate_flag_ = false;
				pico_streaming::ByteRtcMoudel::GetInstance()->right_suggest_rate_ = 0;

			}
}

	}
	
#endif

	mFrameConfigIndex++;
	return configindex;
}

RVR::RVRPoseHmdData StreamingHmdDriver::GetOutputRenderPose()
{
	RVR::RVRPoseHmdData ret = {0};
	current_pose_mutex_.lock();
	ret = out_render_pose_;
	current_pose_mutex_.unlock();
	return ret;
}
void StreamingHmdDriver::SetOutputRenderPose(RVR::RVRPoseHmdData hmd_pose)
{
	 
	current_pose_mutex_.lock();
	out_render_pose_ = hmd_pose;
	out_render_pose_.hmdTimeStamp = hmd_pose.poseTimeStamp;
	current_pose_mutex_.unlock();
}
//-----------------------------------------------------------------------------
void StreamingHmdDriver::PostPresent()
//-----------------------------------------------------------------------------
{
	countPresent++;
	int64_t PresentTime = RVR::nowInNs();
	//RVR_LOG_A("present cost=%f ", (PresentTime - endPresent)/1000000.f);
	if (firstPresent < 0)
	{
		firstPresent = PresentTime;
	}


	//RVR_LOG_A("present intervalt=%f  average=%f", (PresentTime - lastPresent)/1000.f,(PresentTime-firstPresent)/countPresent/1000.f);
	//out_render_pose_.poseRecvTime = GetTimestampUs();

	
	if (((left_encoder_init_ && right_encoder_init_) == false) && (frame_index_ == 720))
	{
		vr::VRServerDriverHost()->RequestRestart("Pico streaming failed to initialize,please Check whether the computer configuration meets the streaming requirements", nullptr, nullptr, nullptr);
	}

	uint64_t CopyStart = RVR::nowInNs();

	last_t = CopyStart;
	bool ret = false;


	ID3D11Texture2D* leftTexture = NULL;
	ID3D11Texture2D* rightTexture = NULL;
	int configindex = 0;
	RVR::RVRPoseHmdData current_render_pose = GetOutputRenderPose();
	current_render_pose.beginEncodeStamp = RVR::nowInNs();
	bool not_encode = false;
	if (current_render_pose.hmdTimeStamp!= last_encode_hmd_timestamps_)
	{
		if (gConfigReader.GetSplit_() == 0)
		{
			if (gPluginManger.mSetPoseCachePtr != NULL)
			{
				current_render_pose.predictedTimeMs = GetDetph_();
				SensorManger::GetInstance()->SaveSensor(current_render_pose);

				gPluginManger.DoSetPoseCache(&current_render_pose);
			}

			configindex = GetEncodeFrameConfig(CopyStart);

			mFrameConfig[configindex].timestamp = CopyStart - current_render_pose.poseRecvTime;
			mFrameConfig[configindex].net_cost = GetInputRenderPose().net_cost;

			if (gLog)
			{
				RVR_LOG_A("steamvr render time cost %lld ", mFrameConfig[configindex].timestamp / 1000000);
			}

		}
		if (gConfigReader.GetNotEncodeValue() == 0)
		{
			if (gConfigReader.BigPicture() == 0)
			{
				leftTexture = compositor.Get(Eye::kLeft);
				rightTexture = compositor.Get(Eye::kRight);
				if (left_encoder_init_ && right_encoder_init_)
				{
					mLeftEnvc->QueueBuffer(leftTexture, &mFrameConfig[configindex]);
					mRightEnvc->QueueBuffer(rightTexture, &mFrameConfig[configindex]);
				}
				if (gPictureTrace)
				{
					wstring filename = L"picture//left" + to_wstring(pictureindex) + L".jpg";
					D3DHelper::SaveTextureToFile(d3d11device_, leftTexture, filename.c_str());
					filename = L"picture//right" + to_wstring(pictureindex) + L".jpg";
					D3DHelper::SaveTextureToFile(d3d11device_, rightTexture, filename.c_str());
					pictureindex++;
				}
			}
			else
			{
				ID3D11Texture2D* savepicture = NULL;
				if (frame_index_ % 2 == 0)
				{
					ID3D11Texture2D* leftTexture = NULL;
					leftTexture = compositor.Get2to1(Eye::kLeft);

					if (left_encoder_init_)
					{
						if (gConfigReader.GetSplit_() == 0)
						{
							mLeftEnvc->QueueBuffer(leftTexture, &mFrameConfig[configindex]);
						}

					}
					savepicture = leftTexture;
				}
				else
				{

					ID3D11Texture2D* rightTexture = NULL;
					rightTexture = compositor.Get2to1(Eye::kRight);

					if (right_encoder_init_)
					{
						if (gConfigReader.GetSplit_() == 0)
						{
#ifndef FORCE_HALFFPS
							if (gConfigReader.GetSingleEncode() == 0)
							{
								mRightEnvc->QueueBuffer(rightTexture, &mFrameConfig[configindex]);
							}
							else
							{
								leftTexture = compositor.Get2to1(Eye::kLeft);
								mLeftEnvc->QueueBuffer(leftTexture, &mFrameConfig[configindex]);
							}
#endif
							
						}
					}
					savepicture = rightTexture;
				}
				if (gPictureTrace)
				{
					wstring filename = L"picture//" + to_wstring(pictureindex) + L".jpg";
					D3DHelper::SaveTextureToFile(d3d11device_, savepicture, filename.c_str());
					pictureindex++;
				}
			}

		}

		if (gLog)
		{
			RVR_LOG_A("all render cost time:%f", float(CopyStart - current_render_pose.poseRecvTime) / 1000000.0f);
		}

		if (gConfigReader.GetSplit_() == 1)
		{
			ID3D11DeviceContext* pContext;
			d3d11device_->GetImmediateContext(&pContext);
			ID3D11Texture2D* split_texture = NULL;
			int index = 0;
			split_texture = compositor.GetSplitTextureForRender(index);
			split_poses_[index] = GetOutputRenderPose();
			IDXGIKeyedMutex* pKeyedMutex = NULL;
			pKeyedMutex = D3DHelper::AcquireMutexByTexture(split_texture);
			if (pKeyedMutex != NULL)
			{
				if (pKeyedMutex->AcquireSync(0, 10) == S_OK)
				{
					if (gConfigReader.BigPicture())
					{
						ID3D11Texture2D* encoderTexture = NULL;
						if (frame_index_ % 2 == 0)
						{
							encoderTexture = compositor.Get2to1(Eye::kLeft);
						}
						else
						{
							encoderTexture = compositor.Get2to1(Eye::kRight);
						}
						D3D11_TEXTURE2D_DESC desc;
						encoderTexture->GetDesc(&desc);


						pContext->CopySubresourceRegion(split_texture,
							0, 0, 0, 0
							, encoderTexture, 0, nullptr);

					}

				}
				D3DHelper::ReleaseMutex(pKeyedMutex);
			};

			compositor.AddSplitRenderIndex();
		}
		//以下是深度检测 
		if (gConfigReader.GetDepthComputeValue() == 1)
		{
			if (depth_inedx_ % 2 == 0)
			{
				ID3D11DeviceContext* pContext;
				d3d11device_->GetImmediateContext(&pContext);
				ID3D11Texture2D* left_shared_texture = NULL;
				ID3D11Texture2D* right_shared_texture = NULL;
				left_shared_texture = compositor.GetSharedTexture(Eye::kLeft);
				right_shared_texture = compositor.GetSharedTexture(Eye::kRight);
				IDXGIKeyedMutex* pKeyedMutex = NULL;
				pKeyedMutex = D3DHelper::AcquireMutexByTexture(left_shared_texture);
				if (pKeyedMutex != NULL)
				{
					if (pKeyedMutex->AcquireSync(0, 10) == S_OK)
					{
						if (gConfigReader.BigPicture())
						{
							ID3D11Texture2D* encoderTexture = NULL;
							if (frame_index_ % 2 == 0)
							{
								encoderTexture = compositor.Get2to1(Eye::kLeft);
							}
							else
							{
								encoderTexture = compositor.Get2to1(Eye::kRight);
							}
							D3D11_TEXTURE2D_DESC desc;
							encoderTexture->GetDesc(&desc);

							D3D11_BOX box;
							box.left = 0;
							box.right = desc.Width / 2;
							box.top = 0;
							box.bottom = desc.Height;
							box.front = 0;
							box.back = 1;
							pContext->CopySubresourceRegion(left_shared_texture,
								0, 0, 0, 0
								, encoderTexture, 0, &box);
						}
						else
						{
							pContext->CopyResource(left_shared_texture, leftTexture);
							pContext->Flush();
						}

					}
					D3DHelper::ReleaseMutex(pKeyedMutex);
				};

				pKeyedMutex = D3DHelper::AcquireMutexByTexture(right_shared_texture);
				if (pKeyedMutex != NULL)
				{
					if (pKeyedMutex->AcquireSync(0, 10) == S_OK)
					{
						if (gConfigReader.BigPicture())
						{
							ID3D11Texture2D* encoderTexture = NULL;
							if (frame_index_ % 2 == 0)
							{
								encoderTexture = compositor.Get2to1(Eye::kLeft);
							}
							else
							{
								encoderTexture = compositor.Get2to1(Eye::kRight);
							}
							D3D11_TEXTURE2D_DESC desc;
							encoderTexture->GetDesc(&desc);

							D3D11_BOX box;
							box.left = desc.Width / 2;
							box.right = desc.Width;
							box.top = 0;
							box.bottom = desc.Height;
							box.front = 0;
							box.back = 1;
							pContext->CopySubresourceRegion(right_shared_texture,
								0, 0, 0, 0
								, encoderTexture, 0, &box);
						}
						else
						{
							pContext->CopyResource(right_shared_texture, rightTexture);
							pContext->Flush();
						}

					}
					D3DHelper::ReleaseMutex(pKeyedMutex);
				}
				if (gLog)
				{
					uint64_t CopyEnd = RVR::nowInUs();
					RVR_LOG_A("copy left and right trace time:%f", float(CopyEnd - CopyStart) / 1000.0f);
				}
				PostThreadMessage(mDepthCalcThreadId, MSG_DEPTH, NULL, NULL);
			}

			depth_inedx_++;
		}
		else
		{
			SetDetph_(200);
		}

		last_encode_hmd_timestamps_ = current_render_pose.hmdTimeStamp;
		frame_index_++;
	}
	else
	{
	not_encode = true;
	}	

	if (gConfigReader.GetSplit_() == 1)
	{
		return;
	}
 
	int64_t add_begin = RVR::nowInNs();
	if ((SensorPasser::GetInstance()->GetNetBadMode_() == false)&&(g_test_sensor_mode!=8))
	{
		SensorPasser::GetInstance()->AddSensorByNotifyEvent(PresentTime, 40);
	}

	int64_t add_end = RVR::nowInNs();
	string msg = " add sensor cost =" + std::to_string((add_end - add_begin) / 1000000);
	
	startFrameTime = RVR::nowInNs();
	int64_t offset = 1000000;
	int64_t intervalt_i64 = per_frame_ns_ - (startFrameTime - lastFrameTime);
	if ((intervalt_i64) > 1000000)
	{
		int64_t sleep_begin = RVR::nowInNs();

		int sleep_time = (intervalt_i64 - offset) / 1000000;
		if ((not_encode==false)&&(sleep_time>0))
		{
			Sleep(sleep_time);

			string msg = "PostPresent sleep=" + std::to_string(sleep_time) + "  intervalt_i64= " + std::to_string(intervalt_i64);
			//RVR_LOG_A(msg.c_str());
		}
		
	}
	startFrameTime = RVR::nowInNs();
	lastFrameTime = startFrameTime;
	lastPresent = PresentTime;
	endPresent = RVR::nowInNs();
	out_render_pose_.beginGameRenderStamp = startFrameTime;
	double vsyn_offset = 0.002;
	VRServerDriverHost()->VsyncEvent(vsyn_offset);
	return;
	
}




//-----------------------------------------------------------------------------
void StreamingHmdDriver::SetOutputRenderPoseFromMatrix34(RVR::RVRPoseHmdData* poseData, const vr::HmdMatrix34_t* pPose)
//-----------------------------------------------------------------------------
{
	 
	current_pose_mutex_.lock();
	ExtractRVRPoseHmdData(poseData, pPose);
	current_pose_mutex_.unlock();
	 
}

