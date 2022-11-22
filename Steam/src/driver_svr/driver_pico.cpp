//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------
#include "SensorSocket.h"
#include "driver_pico.h"
#include "RVRLogger.h"
PicoVRDriver g_svrDriver;
//add by dzhuang
#include "config_reader.h"
#include "openvrtool.h"
#include "filetool.h"
#include "stringtool.h"
#include "PluginManger.h"
#include "Util.h"
#include "PredictPose/picocontroller_interface.h"
#ifndef NO_SLARDAR
#include "SlardarMoudle.h"
#endif  
#include "driver_define.h"
#include "hid_module.h"
#include "sensor_passer.h"
#include "UsbBulkModule.h"
#include <io.h>
#include "Util.h"
#include "../../Eigen/Geometry"
#include "../../Eigen/Core"
#include "sensor_passer.h"
ConfigReader gConfigReader;
PluginManger gPluginManger;
//end
using namespace RVR;
#define POSE_INTERVAL 1

bool gDashboardActivated = false;
bool gOverlayShow = false;
bool gAppConnected = false;
extern uint32_t gBasePid;
extern bool g_test_sensor;
extern int g_test_sensor_mode ;
RVR::RVRPoseHmdData g_test_rvrhmd = { 0 };
extern bool g_long_sensor_;
unsigned int __stdcall PicoVRDriver::SlardarReporterThread(LPVOID lpParameter)
{
	RVR_LOG_A("SlardarReporterThread    Thread");
	PicoVRDriver* driver = (PicoVRDriver*)lpParameter;
#ifndef NO_SLARDAR
	SlardarMoudle::GetInstance()->Init();
	while (!driver->m_bExitEventThread)
	{
		if (SlardarMoudle::GetInstance()->GetMessageBufSize()>= SLARDAR_BUF_SIZE)
		{
			SlardarMoudle::GetInstance()->SendToSlardar();
		}
		Sleep(2);
	}
#endif
	SetEvent(driver->slardar_thread_event_);
	return 1;
}
unsigned int __stdcall PicoVRDriver::AddPoseTogetherThread(LPVOID lpParameter)
{
	PicoVRDriver* driver = (PicoVRDriver*)lpParameter;
	while (!driver->m_bExitEventThread)
	{
		if (gConfigReader.GetSmoothControllerValue() != 2)
		{
			Sleep(1000);
			continue;
		}
		if (g_long_sensor_ ==false)
		{
			Sleep(1000);
			continue;
		}
		RVR::RVRPoseHmdData hmd_pose = { 0 };
		RVR::RVRControllerData  left_controller_pose = { 0 };
		RVR::RVRControllerData right_controller_pose = { 0 };
		driver->sensor_queue_.GetPose(hmd_pose, left_controller_pose, right_controller_pose);
		driver->AddHmdPose(&hmd_pose, false);
		driver->AddControllerPose((uint32_t)ControllerIndex::kLeftController, &left_controller_pose);
		driver->AddControllerPose((uint32_t)ControllerIndex::kRightController, &right_controller_pose);
		Sleep(2);
	}
	SetEvent(driver->add_sensor_together_thread_event_);
	return 1;
}
unsigned int __stdcall PicoVRDriver::AddHmdPoseThread(LPVOID lpParameter)
{
	 
	int64_t base_hmd_time = -1;
	RVR_LOG_A("AddHmdPose    Thread"); 
	PicoVRDriver* driver = (PicoVRDriver*)lpParameter;
	int64_t last_use_timestamp = -1;
	RVR::RVRPoseHmdData last_hmd = {0};
	int64_t last_pre_locate_time = -1;
	int64_t ttsdk = 0;
	int64_t ttlo = 0;
	int64_t little_count = 0;
	int64_t big_count = 0;
	int64_t last_last_locate_time_ = -1;
	int64_t socket_recv_space = 0;
	int64_t last_last_sdk_time = -1;
	while (!driver->m_bExitEventThread)
	{
		if (((SensorSocket*)SensorSocket::GetInstance(-1))->base_sdk_time_ < 0||driver->GetHmdActiveType_()== (int)Hmd_Active_Type::DP
			|| gConfigReader.GetRtcOrBulkMode_() !=0||gConfigReader.GetSmoothControllerValue()!=1)
		{
			Sleep(1);
			continue;
		}
		 
		RVR::RVRPoseHmdData hmd_pose = {0};
		SensorPasser::GetInstance()->GetHmdSensor(hmd_pose);
		int64_t last_locate_time_ = hmd_pose.poseRecvTime;
		int64_t sdk_timestamp = hmd_pose.poseTimeStamp;
		 
		if (last_last_locate_time_==-1)
		{
			last_last_locate_time_ = last_locate_time_;
			last_last_sdk_time = sdk_timestamp;
		}
		if (last_last_locate_time_!=last_locate_time_)
		{
			if ((last_locate_time_-last_last_locate_time_)>20000000)
			{
				big_count++;
				//DriverLog("hmd sensor recvspace=%lld sdkspace=%lld rate=%lf", last_locate_time_ - last_last_locate_time_,sdk_timestamp-last_last_sdk_time, (double)big_count / (double)(big_count + little_count));
			}
			else
			{
				little_count++;
				 
			}
			socket_recv_space = last_locate_time_ - last_last_locate_time_;
			if ((socket_recv_space - (sdk_timestamp - last_last_sdk_time)) > 15000000)
			{
				RVR_LOG_A("real recv space too big  %lld", socket_recv_space - (sdk_timestamp - last_last_sdk_time));
			}

			last_last_locate_time_ = last_locate_time_;
			last_last_sdk_time = sdk_timestamp;
		}
		
		algo_result_t origion_sensor = { 0 };
		algo_result_t predicted_sensor = { 0 };

		int64_t timestamp = RVR::nowInNs();
		if (base_hmd_time < 0)
		{
			base_hmd_time = timestamp;
		}

		int64_t use_timestamp = sdk_timestamp + timestamp - last_locate_time_;
		
		if ((use_timestamp < last_use_timestamp)&& (socket_recv_space >10000000))//时间戳比上个小   并且 接收间隔超过 10 ms（意味着sensor在网络上慢了。之前使用了预测值，时域上已经超前于当前sensor）
		{
			if (last_hmd.valid == true)
			{
				hmd_pose = last_hmd;
				//DriverLog("hmd not pred sub sdk=%lld sub locate=%lld sub-sub=%lld sub=%lld", sdk_timestamp - ttsdk, timestamp - ttlo, sdk_timestamp - ttsdk- timestamp +ttlo, use_timestamp - last_use_timestamp);
			}
		}
		else //时间戳比上个大，预测
		{
			last_pre_locate_time = timestamp;
			
			ConvertRvrHmdPoseToPredict(hmd_pose, sdk_timestamp, origion_sensor);

			double timestamp_d = use_timestamp;

			PredictMotion(origion_sensor, timestamp_d, predicted_sensor);
			ConvertPoseToRvrHmdPose(predicted_sensor, hmd_pose);

			last_hmd = hmd_pose;
			last_use_timestamp = use_timestamp;
			//DriverLog("hmd sdk_t=%lld ues_t=%lld sub=%lld", sdk_timestamp, use_timestamp, use_timestamp - sdk_timestamp);
		}
		ttsdk = sdk_timestamp;
		ttlo = timestamp;

		driver->AddHmdPose(&hmd_pose);

		Sleep(POSE_INTERVAL);
	}
	SetEvent(g_svrDriver.add_hmd_thread_event_);
	return 1;
};
unsigned int __stdcall PicoVRDriver::AddControllerPoseThread(LPVOID lpParameter)
{
	uint32_t* controller_index_ptr = (uint32_t*)lpParameter;
	uint32_t controller_index = *controller_index_ptr;
	 
	int64_t base_left_time = -1;
	PicoVRDriver* driver = (PicoVRDriver*)&g_svrDriver;
	int64_t last_time = -1;
	RVR_LOG_A("AddLeftControllerPose      Thread");
	RVR::RVRControllerData  last_add_pose = {0};
	int64_t last_pre_locate_time = -1;
	int64_t last_last_locate_time_ = -1;
	int64_t socket_recv_space = 0;
	int64_t little_count = 0;
	int64_t big_count = 0;
	int64_t last_last_sdk_time = -1;
	while (!driver->m_bExitEventThread)
	{
		//RVR_LOG_A("AddLeftControllerPoseThread");
		if (((ControllerSensorSocket*)SensorSocket::GetInstance(controller_index))->base_sdk_time_ < 0 || driver->GetHmdActiveType_() == (int)Hmd_Active_Type::DP
			|| gConfigReader.GetRtcOrBulkMode_() != 0||gConfigReader.GetSmoothControllerValue() != 1)
		{
			Sleep(1);
			continue;
		}
		RVR::RVRControllerData add_pose = {0};
		
		SensorPasser::GetInstance()->GetControllerSensor(add_pose, (ControllerIndex)controller_index);
		int64_t last_locate_time_ = add_pose.recvTimeStampNs;
		int64_t sdk_timestamp = add_pose.timestamp;;
	
		if (last_last_locate_time_ == -1)
		{
			last_last_locate_time_ = last_locate_time_;
			last_last_sdk_time = sdk_timestamp;
		}
		if (last_last_locate_time_ != last_locate_time_)
		{
			if ((last_locate_time_ - last_last_locate_time_) > 20000000)
			{
				big_count++;
				//DriverLog("controller %d sensor recvspace=%lld sdkspce=%lld rate=%lf", controller_index,last_locate_time_ - last_last_locate_time_,sdk_timestamp-last_last_sdk_time, (double)big_count / (double)(big_count + little_count));
			}
			else
			{
				little_count++;

			}
			socket_recv_space = last_locate_time_ - last_last_locate_time_;
			last_last_locate_time_ = last_locate_time_;
			last_last_sdk_time = sdk_timestamp;
		}
		algo_result_t origion_sensor = { 0 };
		algo_result_t predicted_sensor = { 0 };

		int64_t timestamp = RVR::nowInNs();
		if (base_left_time < 0)
		{
			base_left_time = timestamp;
		}

		int64_t use_timestamp = sdk_timestamp + timestamp - last_locate_time_;
		if ((use_timestamp < last_time) && (socket_recv_space > 10000000))//时间戳比上个小，不预测
		{
			if (add_pose.connectionState==RVR::RVRControllerConnectionState::kConnected)
			{
				add_pose = last_add_pose;
			}
			
			//DriverLog("left not pred sdk_t=%lld ues_t=%lld sub=%lld", sdk_timestamp, use_timestamp, use_timestamp - sdk_timestamp);
		}
		else //时间戳比上个大，预测
		{
			last_pre_locate_time = timestamp;
			ConvertRvrControllerPoseToPredict(add_pose, sdk_timestamp, origion_sensor);
			double timestamp_d = use_timestamp;
			PredictMotion(origion_sensor, timestamp_d, predicted_sensor);
			ConvertPoseToRvrControllerPose(predicted_sensor, add_pose);
			last_add_pose = add_pose;
			last_time = use_timestamp;
			//DriverLog("left sdk_t=%lld ues_t=%lld sub=%lld", sdk_timestamp, use_timestamp, use_timestamp - sdk_timestamp);
		}
		if (driver->GetHmdActiveType_() == (int)Hmd_Active_Type::DP)
		{
			driver->AddControllerPose(controller_index, &add_pose);
		}
		

		Sleep(POSE_INTERVAL);
	}
	SetEvent(g_svrDriver.add_controller_thread_event_[controller_index]);
	return 1;
};

void set_test_basepose(DriverPose_t &base_pose)
{
	if (base_pose.poseIsValid == false)
	{
		base_pose.poseIsValid = true;
		base_pose.result = vr::TrackingResult_Running_OK;
		for (int i = 0; i < 3; i++)
		{
			base_pose.vecWorldFromDriverTranslation[i] = 0.0;
			base_pose.vecDriverFromHeadTranslation[i] = 0.0;
		}

		base_pose.qRotation.w = 0.999;
		base_pose.qRotation.x = 0.026;
		base_pose.qRotation.y = 0.017;
		base_pose.qRotation.z = 0.009;

		base_pose.qWorldFromDriverRotation.w = 1;
		base_pose.qWorldFromDriverRotation.x = 0;
		base_pose.qWorldFromDriverRotation.y = 0;
		base_pose.qWorldFromDriverRotation.z = 0;

		base_pose.qDriverFromHeadRotation.w = 1;
		base_pose.qDriverFromHeadRotation.x = 0;
		base_pose.qDriverFromHeadRotation.y = 0;
		base_pose.qDriverFromHeadRotation.z = 0;

		// some things are always true
		base_pose.shouldApplyHeadModel = false;// true;
		base_pose.willDriftInYaw = false;
		base_pose.poseTimeOffset = 0.001;
		// we don't do position, so these are easy
		for (int i = 0; i < 3; i++)
		{
			base_pose.vecPosition[i] = 0.0;
			base_pose.vecVelocity[i] = 0.0;
			base_pose.vecAcceleration[i] = 0.0;

			// we also don't know the angular velocity or acceleration
			base_pose.vecAngularVelocity[i] = 0.0;
			base_pose.vecAngularAcceleration[i] = 0.0;

		}
		base_pose.deviceIsConnected = true;
	}
	base_pose.vecPosition[1] = 1.3;
}
 
#include<fstream>
void FileSensorTest()
{
	vr::DriverPose_t hmd_pose = {0};
	hmd_pose.qDriverFromHeadRotation.w = 1;
	hmd_pose.qWorldFromDriverRotation.w = 1;
	hmd_pose.deviceIsConnected = true;
	hmd_pose.poseIsValid = true;
	hmd_pose.result = vr::TrackingResult_Running_OK;
	vr::DriverPose_t left_pose = hmd_pose;
	vr::DriverPose_t right_pose = hmd_pose;
	
	std::string line;
	std::ifstream in("D:\\sensor.txt", std::ios::in);
	int64_t last_ts = -1;
	int substrlen = strlen("sensertest:");
	int add_time = 0;
	if (in)
	{
		
		while (1)
		{
			int64_t read_begin = RVR::nowInNs();
			if (!getline(in, line))
			{
				break;
			}
			std::string work_str = line;
			work_str = work_str.substr(work_str.find("sensertest:") + substrlen, work_str.length() - work_str.find("sensertest:") - substrlen);
			hmd_pose.qRotation.w = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			hmd_pose.qRotation.x = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			hmd_pose.qRotation.y = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			hmd_pose.qRotation.z = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);

			hmd_pose.vecPosition[0] = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			hmd_pose.vecPosition[1] = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			hmd_pose.vecPosition[2] = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);


			left_pose.qRotation.w = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			left_pose.qRotation.x = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			left_pose.qRotation.y = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			left_pose.qRotation.z = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);

			left_pose.vecPosition[0] = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			left_pose.vecPosition[1] = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			left_pose.vecPosition[2] = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);



			right_pose.qRotation.w = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			right_pose.qRotation.x = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			right_pose.qRotation.y = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			right_pose.qRotation.z = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);

			right_pose.vecPosition[0] = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			right_pose.vecPosition[1] = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());
			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);
			right_pose.vecPosition[2] = atof(work_str.substr(0, work_str.find_first_of(",")).c_str());

			work_str = work_str.substr(work_str.find_first_of(",") + 1, work_str.length() - work_str.find_first_of(",") - 1);

			int64_t ts = atoll(work_str.substr(0, work_str.find_first_of(",")).c_str());

			int64_t read_end = RVR::nowInNs();
			if (last_ts > 0)
			{
				int sleep_time = (ts - last_ts + 500000+read_begin- read_end) / 1000000;
				if (sleep_time<0)
				{
					RVR_LOG_A("catch");
				}
				else
				{
					Sleep(sleep_time);
				}
			
			}
			last_ts = ts;
			
			if (g_svrDriver.GetHmdActiveType_() == (int)Hmd_Active_Type::Streaming)
			{

			vr:VRServerDriverHost()->TrackedDevicePoseUpdated(g_svrDriver.GetStreamingHmdDriver()->GetVrDeviceId(), hmd_pose, sizeof(DriverPose_t));

			}
			else
			{
				if (g_svrDriver.GetStreamingHmdDriver()->GetIsAdded_() == false)
				{
					bool result = vr::VRServerDriverHost()->TrackedDeviceAdded(g_svrDriver.GetStreamingHmdDriver()->GetSerialNumber().c_str(), vr::TrackedDeviceClass_HMD, g_svrDriver.GetStreamingHmdDriver());
					g_svrDriver.SetHmdActiveType_((int)Hmd_Active_Type::Streaming);
					DriverLog("add pico_svr_hmd");
					if (result)
					{
						g_svrDriver.GetStreamingHmdDriver()->SetIsAdded_(true);
					}

				}
			}

			if (g_svrDriver.GetController((int)ControllerIndex::kLeftController)->GetIsActived_())
			{

				vr::VRServerDriverHost()->TrackedDevicePoseUpdated(g_svrDriver.GetController((int)ControllerIndex::kLeftController)->GetDriverObjId(), left_pose, sizeof(DriverPose_t));


			}
			else
			{
				bool result = vr::VRServerDriverHost()->TrackedDeviceAdded(g_svrDriver.GetController((int)ControllerIndex::kLeftController)->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, g_svrDriver.GetController((int)ControllerIndex::kLeftController));
				if (result)
				{
					g_svrDriver.GetController((int)ControllerIndex::kLeftController)->SetIsAdded_(true);
				}

			}
			if (g_svrDriver.GetController((int)ControllerIndex::kRightController)->GetIsActived_())
			{

				vr::VRServerDriverHost()->TrackedDevicePoseUpdated(g_svrDriver.GetController((int)ControllerIndex::kRightController)->GetDriverObjId(), right_pose, sizeof(DriverPose_t));

			}
			else
			{
				bool result = vr::VRServerDriverHost()->TrackedDeviceAdded(g_svrDriver.GetController((int)ControllerIndex::kRightController)->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, g_svrDriver.GetController((int)ControllerIndex::kRightController));
				if (result)
				{
					g_svrDriver.GetController((int)ControllerIndex::kRightController)->SetIsAdded_(true);
				}

			}
			RVR::RVRPoseHmdData hmd = {0};
			RVR::RVRControllerData left = { 0 };
			RVR::RVRControllerData right = { 0 };
			ExtractRVRPoseHmdData(&hmd_pose, &hmd);
			ExtractRVRControllerPoseData(&left_pose, &left);
			ExtractRVRControllerPoseData(&right_pose, &right);
			//SensorPasser::GetInstance()->SetAllSensor(hmd,left,right);
			///*printf("%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
			//	w, x, y, z, px, py, pz,
			//	wl, xl, yl, zl, pxl, pyl, pzl,
			//	wr, xr, yr, zr, pxr, pyr, pzr);*/
		}
	}

}

unsigned int __stdcall PicoVRDriver::TestSensorThread(LPVOID lpParameter)
{


	vr::DriverPose_t base_pose = g_svrDriver.GetStreamingHmdDriver()->GetPose();
	set_test_basepose(base_pose);
	base_pose.qRotation.w = 1;
	base_pose.qRotation.x = base_pose.qRotation.y = base_pose.qRotation.z = 0;
	ExtractRVRPoseHmdData(&base_pose, &g_test_rvrhmd);
	base_pose.poseTimeOffset = 0;
	base_pose.result= vr::TrackingResult_Running_OK;
	base_pose.poseIsValid = true;
	base_pose.deviceIsConnected = true;
	vr::DriverPose_t hmd_pose = base_pose;
	vr::DriverPose_t left_pose = base_pose;
	left_pose.vecPosition[2] = left_pose.vecPosition[2] - 0.6;
	left_pose.vecPosition[1] = left_pose.vecPosition[1] -2.15;
	left_pose.vecPosition[0] = left_pose.vecPosition[0] - 0.2;
	vr::DriverPose_t right_pose = base_pose;
	right_pose.vecPosition[2] = right_pose.vecPosition[2] - 0.6;
	right_pose.vecPosition[1] = right_pose.vecPosition[1] - 0.15;
	right_pose.vecPosition[0] = right_pose.vecPosition[0] + 0.2;
	int sleep_time =11;
	float base_v = 0.35f;
	float left_add_x = base_v * (float)sleep_time / 1000.f;
	float right_add_x = base_v * (float)sleep_time / 1000.f;
	float hmd_add_x = 0;
	float hmd_add_v = hmd_add_x * sleep_time / 1000.f;
	base_pose.vecVelocity[0] = 0;
	hmd_pose.vecVelocity[0] = 0;

	Eigen::Vector3d hmd_angle = {0,0,0};
	float angle_speed = 25.f/1000.f*(float)sleep_time;
	float add_flag = 1;
	int add_index = 0;
	int64_t last_add = 0;
	float direction_flag = 1;

	float r_x = 0;
	float r_z = 0;
	while (g_test_sensor&&(!g_svrDriver.m_bExitEventThread))
	{
		
		 left_add_x = direction_flag* base_v * (float)sleep_time / 1000.f;
		 right_add_x = direction_flag * base_v * (float)sleep_time / 1000.f;
		 left_pose.vecVelocity[0] =- direction_flag * base_v;
		 right_pose.vecVelocity[0] = direction_flag * base_v;

		 if (g_test_sensor_mode==0)
		 {
			  hmd_add_x = 0;
			  hmd_add_v =0;
		 }else if (g_test_sensor_mode==3)
		 {
			
			 hmd_add_x = direction_flag*base_v * (float)sleep_time / 1000.f;;
			 hmd_add_v = direction_flag * base_v;
			 hmd_pose.vecVelocity[0] = direction_flag * base_v;
		 }
		if (g_test_sensor_mode==8)
		{
			FileSensorTest();
			continue;
		}
		if (g_test_sensor_mode==7)// 头 手同轴转动
		{
			hmd_pose.vecAngularVelocity[1] = 6 * 60.f / 1000.f * add_flag * DEGREES_TO_RADIANS;
			left_pose.vecAngularVelocity[1] = 3;
			left_pose.vecAngularVelocity[0] = 3;
			right_pose.vecAngularVelocity[0] = 6;
			hmd_angle[1] = hmd_angle[1] + angle_speed * add_flag;
			if (fabs(hmd_angle[1]) > 45)
			{
				add_flag = add_flag * (-1);
			}
			Eigen::Vector3d hmd_raduabs = { 0,hmd_angle[1] * DEGREES_TO_RADIANS,0 };
			Eigen::AngleAxisd rollAngle(Eigen::AngleAxisd(hmd_raduabs(0), Eigen::Vector3d::UnitX()));
			Eigen::AngleAxisd pitchAngle(Eigen::AngleAxisd(hmd_raduabs(1), Eigen::Vector3d::UnitY()));
			Eigen::AngleAxisd yawAngle(Eigen::AngleAxisd(hmd_raduabs(2), Eigen::Vector3d::UnitZ()));

			Eigen::Quaterniond hmd_q;
			hmd_q = yawAngle * pitchAngle * rollAngle;
			hmd_pose.qRotation.w = hmd_q.w();
			hmd_pose.qRotation.x = hmd_q.x();
			hmd_pose.qRotation.y = hmd_q.y();
			hmd_pose.qRotation.z = hmd_q.z();

			Eigen::Vector3d e_left_add = Eigen::Vector3d(-0.2, -0.15, -0.54);
			Eigen::Vector3d e_right_add = Eigen::Vector3d(0.2, -0.15, -0.54);

			Eigen::Vector3d e_left_trans = hmd_q * e_left_add;
			Eigen::Vector3d e_right_trans = hmd_q * e_right_add;
			left_pose = hmd_pose;
			right_pose = hmd_pose;
			left_pose.vecPosition[0] = left_pose.vecPosition[0] + e_left_trans.x();
			left_pose.vecPosition[1] = left_pose.vecPosition[1] + e_left_trans.y();
			left_pose.vecPosition[2] = left_pose.vecPosition[2] + e_left_trans.z();

			right_pose.vecPosition[0] = right_pose.vecPosition[0] + e_right_trans.x();
			right_pose.vecPosition[1] = right_pose.vecPosition[1] + e_right_trans.y();
			right_pose.vecPosition[2] = right_pose.vecPosition[2] + e_right_trans.z();

			left_pose.qRotation.w = hmd_q.w();
			left_pose.qRotation.x = hmd_q.x();
			left_pose.qRotation.y = hmd_q.y();
			left_pose.qRotation.z = hmd_q.z();

			right_pose.qRotation.w = hmd_q.w();
			right_pose.qRotation.x = hmd_q.x();
			right_pose.qRotation.y = hmd_q.y();
			right_pose.qRotation.z = hmd_q.z();

		}

		else{
			hmd_pose.vecPosition[0] = hmd_pose.vecPosition[0] + hmd_add_x;

			hmd_pose.vecVelocity[0] = hmd_add_v;
			
			left_pose.vecPosition[2] = left_pose.vecPosition[2] ;
			left_pose.vecPosition[1] = left_pose.vecPosition[1]  ;
			left_pose.vecPosition[0] = left_pose.vecPosition[0]  + left_add_x;
		 
			right_pose.vecPosition[2] = right_pose.vecPosition[2] ;
			right_pose.vecPosition[1] = right_pose.vecPosition[1];
			right_pose.vecPosition[0] = right_pose.vecPosition[0] + left_add_x;
			 
		}
		if (g_test_sensor_mode>=0)
		{
			if (GetAsyncKeyState('B') != 0)
			{
				base_pose = g_svrDriver.GetStreamingHmdDriver()->GetPose();
				set_test_basepose(base_pose);
				 
				ExtractRVRPoseHmdData(&base_pose, &g_test_rvrhmd);
				hmd_pose = left_pose = base_pose;
				left_pose.vecPosition[2] = left_pose.vecPosition[2] - 0.6;
				left_pose.vecPosition[1] = left_pose.vecPosition[1] - 0.15;
				left_pose.vecPosition[0] = left_pose.vecPosition[0] - 0.2;
				right_pose = base_pose;
				right_pose.vecPosition[2] = right_pose.vecPosition[2] - 0.6;
				right_pose.vecPosition[1] = right_pose.vecPosition[1] - 0.15;
				right_pose.vecPosition[0] = right_pose.vecPosition[0] + 0.2;
				right_pose.willDriftInYaw = false;
				right_pose.poseTimeOffset = 0.009;
			}
		
			if (g_svrDriver.GetStreamingHmdDriver()->GetIsAdded_() == true)
			{
			    
					RVR::RVRPoseHmdData hmd = { 0 };
					ExtractRVRPoseHmdData(&hmd_pose, &hmd);
					RVR::RVRControllerData left = { 0 };
					ExtractRVRControllerPoseData(&left_pose, &left);

					RVR::RVRControllerData right = { 0 };
					ExtractRVRControllerPoseData(&right_pose, &right);
					
					hmd.poseTimeStamp = hmd.hmdTimeStamp = add_index;
					SensorPasser::GetInstance()->SetAllSensor(hmd, left, right);
					int64_t nts = RVR::nowInNs();
					char msg[2048] = { 0 };
					sprintf_s(msg, "SetAllSensor rotation %lf,%lf,%lf,%lf,  pose %lf,%lf,%lf vx=%f ts=%lld\n",
						hmd.rotation.w, hmd.rotation.x, hmd.rotation.y, hmd.rotation.z,
						hmd.position.x, hmd.position.y, hmd.position.z,hmd.linearVelocity.x,(nts-last_add)/1000000);
				  
					RVR_LOG_A(msg);
					last_add = nts;
				//}
				//else
				//{
				//	int64_t nts = RVR::nowInNs();
				//   vr:VRServerDriverHost()->TrackedDevicePoseUpdated(g_svrDriver.GetStreamingHmdDriver()->GetVrDeviceId(), hmd_pose, sizeof(DriverPose_t));
				//	char msg[2048] = { 0 };
				//	sprintf_s(msg, "add rotation %lf,%lf,%lf,%lf,  pose %lf,%lf,%lf  vx=%f ts=%lld\n",
				//		hmd_pose.qRotation.w, hmd_pose.qRotation.x, hmd_pose.qRotation.y, hmd_pose.qRotation.z,
				//		hmd_pose.vecPosition[0], hmd_pose.vecPosition[1], hmd_pose.vecPosition[2],hmd_pose.vecVelocity[0], (nts - last_add) / 1000000);

				//	//RVR_LOG_A(msg);
				//	last_add = nts;
				//}
			}
			else
			{
				if (g_svrDriver.GetStreamingHmdDriver()->GetIsAdded_() == false  )
				{
					bool result = vr::VRServerDriverHost()->TrackedDeviceAdded(g_svrDriver.GetStreamingHmdDriver()->GetSerialNumber().c_str(), vr::TrackedDeviceClass_HMD, g_svrDriver.GetStreamingHmdDriver());
					g_svrDriver.SetHmdActiveType_((int)Hmd_Active_Type::Streaming);
					DriverLog("add pico_svr_hmd");
					if (result)
					{
						g_svrDriver.GetStreamingHmdDriver()->SetIsAdded_(true);
					}

				}
			}
		}
		if (g_svrDriver.GetController((int)ControllerIndex::kLeftController)->GetIsActived_())
		{
			
		
		}
		else
		{
			bool result = vr::VRServerDriverHost()->TrackedDeviceAdded(g_svrDriver.GetController((int)ControllerIndex::kLeftController)->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, g_svrDriver.GetController((int)ControllerIndex::kLeftController));
			if (result)
			{
				g_svrDriver.GetController((int)ControllerIndex::kLeftController)->SetIsAdded_(true);
			}

		}
		if (g_svrDriver.GetController((int)ControllerIndex::kRightController)->GetIsActived_())
		{
			
		}
		else
		{
			bool result = vr::VRServerDriverHost()->TrackedDeviceAdded(g_svrDriver.GetController((int)ControllerIndex::kRightController)->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, g_svrDriver.GetController((int)ControllerIndex::kRightController));
			if (result)
			{
				g_svrDriver.GetController((int)ControllerIndex::kRightController)->SetIsAdded_(true);
			}

		}

		Sleep(sleep_time);
		add_index++;
		if (g_test_sensor_mode==7)
		{
			continue;
		}
		if (g_test_sensor_mode == 4)
		{
			SensorPasser::GetInstance()->SetNewSensorNotifyEvent();
		}
	    if (g_test_sensor_mode==0)
	    {
			if (fabs(left_pose.vecPosition[0] - base_pose.vecPosition[0]) > 0.4f)
			{
				direction_flag = -direction_flag;
			}

	    }
		

		
			
		if (fabs(hmd_pose.vecPosition[0] - base_pose.vecPosition[0]) > 0.4f)
		{	
			direction_flag = -direction_flag;
		}
			
		
	}
	return 1;
}
//-----------------------------------------------------------------------------
EVRInitError PicoVRDriver::Init(vr::IVRDriverContext *pDriverContext)
//-----------------------------------------------------------------------------
{
	//Sleep(7000);
	CoInitialize(NULL);
	VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
	InitDriverLog(vr::VRDriverLog());
//	direct_mode_ = new GpuDirectMode();
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		
	}
	char str[256];
	wchar_t* wstr = reinterpret_cast<wchar_t*>(str);
	GetSystemDirectory(wstr, 255);
	std::wstring pathStr = wstr;

	pathStr.replace(1, pathStr.length() - 1, L"");
	DWORD len = 255;
	GetUserName(wstr, &len);
	pathStr = pathStr + L":\\Users\\" + wstr;
	std::wstring userpath_rvr = pathStr + L"\\AppData\\Local\\Pico Link\\RVRPlugin.ini";
	std::string userpath_config = WString2String(userpath_rvr);
	DriverLog("config ini:%s", userpath_config.c_str());

	string configPath;
	char driverpath[1024] = { 0 };
	GetSelfModulePath(driverpath);
	driver_folder_ = driverpath;
	deletesub(driver_folder_, "driver_pico.dll", strlen("driver_pico.dll"));

	configPath = driver_folder_ + "RVRPlugin.ini";

	/*if ((_access(userpath_config.c_str(), 0) != -1))
	{

		DriverLog("configpath %s", userpath_config.c_str());

		gConfigReader.ReadConfig(userpath_config);
	}
	else*/
	{

		DriverLog("configpath %s", configPath.c_str());

		gConfigReader.ReadConfig(configPath);
	}

	
	DriverLog("controller type is %d", gConfigReader.GetControllerType());
	string VEncdll = driverpath;
	deletesub(VEncdll, "driver_pico.dll", strlen("driver_pico.dll"));
	VEncdll += "VEncPlugin.dll";
	DriverLog("VEncdll %s", VEncdll.c_str());
	if (gPluginManger.LoadPlugin(VEncdll))
	{
		gPluginManger.GetFunPtrOfSetPoseCache();
		gPluginManger.GetFunPtrOfCreateVEncPlugin();
		gPluginManger.GetFunPtrOfSetDstIpPlugin();
		gPluginManger.GetFunPtrOfSetPoseDepthPlugin();
		gPluginManger.GetFunPtrOfRegistLogFun();
		gPluginManger.GetFunPtrOfRegistPushEncodedFrame();
		DriverLog("VEncdll Load");
	}
	//end 
    m_PoseMode = RVR::RVRPoseMode::RemotePoseFromHMD;

    D3DHelper::CreateDevice(d3d11Device);
	 
    mStubInstance = RVRStub::Instance();
    if (mStubInstance)
    {
#ifndef picomodel
		mStubInstance->Init(NULL);
#else
		mStubInstance->Init(d3d11Device);
#endif
         
    }
    streaming_hmd_ = new StreamingHmdDriver(mStubInstance, d3d11Device);   
	dp_hmd_ = new DpHmdDriver();
    // Controller 0 
    controller_[(int)ControllerIndex::kLeftController] = new ControllerDriver(mStubInstance,(int) ControllerIndex::kLeftController);
    //Controller 1  
    controller_[(int)ControllerIndex::kRightController] = new ControllerDriver(mStubInstance, (int)ControllerIndex::kRightController);
    {		
		//vr::VRServerDriverHost()->TrackedDeviceAdded(/*"LHR-D86EED16"*/"PicoController0", vr::TrackedDeviceClass_Controller, controller_[0]);
		//vr::VRServerDriverHost()->TrackedDeviceAdded(/*"LHR-5BC03915"*/"PicoController1", vr::TrackedDeviceClass_Controller, controller_[1]);
		//end      
		mStubInstance->StartPoseRecv();
		m_hEventThread = CreateThread(nullptr, 0, EventPollThread, (LPVOID)this, 0, nullptr);
		m_hPicoPanelThread = CreateThread(nullptr, 0, PicoPanelProtecterThread, (LPVOID)this, 0, nullptr);
    }

	slardar_thread_event_= CreateEvent(NULL, FALSE, FALSE, NULL);
	add_hmd_thread_event_= CreateEvent(NULL, FALSE, FALSE, NULL);
	add_sensor_together_thread_event_= CreateEvent(NULL, FALSE, FALSE, NULL);
	for (int i=0;i<2;i++)
	{
		add_controller_thread_event_[i]= CreateEvent(NULL, FALSE, FALSE, NULL);
	}
	_beginthreadex(NULL, 0, &SlardarReporterThread, this, 0, 0);
	_beginthreadex(NULL, 0, &AddHmdPoseThread, this, 0, 0);
	_beginthreadex(NULL, 0, &AddControllerPoseThread, controller_[0]->GetControllerIndexPtr(), 0, 0);
	_beginthreadex(NULL, 0, &AddControllerPoseThread, controller_[1]->GetControllerIndexPtr(), 0, 0);
	_beginthreadex(NULL, 0, &AddPoseTogetherThread, this, 0, 0);
	return VRInitError_None;
}

//-----------------------------------------------------------------------------
void PicoVRDriver::Cleanup()
//-----------------------------------------------------------------------------
{
	RVRStub::Instance()->ShutDown();
    if (m_hEventThread != INVALID_HANDLE_VALUE) {
        m_bExitEventThread = true;
        WaitForSingleObject(m_hEventThread, INFINITE);
    }
	
	if (m_hPicoPanelThread != INVALID_HANDLE_VALUE) {
		 
		WaitForSingleObject(m_hPicoPanelThread, INFINITE);
	}
	if (add_hmd_thread_event_ != INVALID_HANDLE_VALUE)
	WaitForSingleObject(add_hmd_thread_event_, INFINITE);

	if (add_controller_thread_event_[0] != INVALID_HANDLE_VALUE)
	WaitForSingleObject(add_controller_thread_event_[0], INFINITE);

	if (add_controller_thread_event_[1] != INVALID_HANDLE_VALUE)
	WaitForSingleObject(add_controller_thread_event_[1], INFINITE);

	if (slardar_thread_event_ != INVALID_HANDLE_VALUE)
	WaitForSingleObject(slardar_thread_event_, INFINITE);

	if (add_sensor_together_thread_event_ != INVALID_HANDLE_VALUE)
	WaitForSingleObject(add_sensor_together_thread_event_, INFINITE);
    if (controller_[0] != nullptr)
    {
        delete controller_[0];
        controller_[0] = nullptr;
    }

    if (controller_[1] != nullptr)
    {
        delete controller_[1];
        controller_[1] = nullptr;
    }

	if (streaming_hmd_ != nullptr)
	{
		streaming_hmd_->Cleanup();
		
	}
	
	if (dp_hmd_!=nullptr)
	{
		HidModule::GetInstance()->CleanUp();
		dp_hmd_->Cleanup();
		
	}
	if (streaming_hmd_)
	{
		delete streaming_hmd_;
		streaming_hmd_ = nullptr;
	}
	if (dp_hmd_)
	{
		delete dp_hmd_;
		dp_hmd_ = nullptr;
	}
	DriverLog("pico drvier cleanup");
	CleanupDriverLog();
}

//-----------------------------------------------------------------------------
void PicoVRDriver::RunFrame()
//-----------------------------------------------------------------------------
{
    streaming_hmd_->RunFrame();
}


void PicoVRDriver::AddHmdPose(RVR::RVRPoseHmdData* pose, bool dp)
{
	if (dp == false)
	{
		if (dp_hmd_->GetIsActived_())//dp 模式已经激活，屏蔽无线
		{
			return;
		}
		if (streaming_hmd_->GetIsAdded_() == false && pose->valid)
		{
			bool result = vr::VRServerDriverHost()->TrackedDeviceAdded(streaming_hmd_->GetSerialNumber().c_str(), vr::TrackedDeviceClass_HMD, streaming_hmd_);
			SetHmdActiveType_((int)Hmd_Active_Type::Streaming);
			DriverLog("add pico_svr_hmd");
			if (result)
			{
				streaming_hmd_->SetIsAdded_(true);
			}

		}
		if (streaming_hmd_->GetIsActived_())
		{
			streaming_hmd_->UpdatePose(pose);
		}
	}
	else
	{
		if (streaming_hmd_->GetIsActived_())//无线 模式已经激活，屏蔽dp
		{
			return;
		}
		if (dp_hmd_->GetIsAdded_() == false && pose->valid)
		{
			/*direct_mode_->SetDirectMode(DisplayEdid, true);
			while (direct_mode_->GetDisplayConnected(DisplayEdid) == false)
			{
				Sleep(1);
			}
			delete direct_mode_;*/
			bool result = false;
#ifndef NO_DP
			result = vr::VRServerDriverHost()->TrackedDeviceAdded(dp_hmd_->GetSerialNumber_().c_str(), vr::TrackedDeviceClass_HMD, dp_hmd_);
#endif
			
			SetHmdActiveType_((int)Hmd_Active_Type::DP);
			DriverLog("add pico_dp_hmd");
			if (result)
			{
				dp_hmd_->SetIsAdded_(true);
			}
			
		}
		if (dp_hmd_->GetIsActived_())
		{
			dp_hmd_->UpdatePose(pose);
		}


	}
}
void PicoVRDriver::AddControllerPose(uint32_t index, RVR::RVRControllerData* pose, HidType::ButtonStateGather button_state)
{
	//end 
	if (controller_[0] == NULL || controller_[1] == NULL)
	{
		return;
	}
	if (index < 2)
	{
		if (controller_[index]->GetIsAdded_()==false &&pose->connectionState== RVR::RVRControllerConnectionState::kConnected)
		{		 
			 
			bool result=vr::VRServerDriverHost()->TrackedDeviceAdded(controller_[index]->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, controller_[index]);
			if (result) 
			{
				controller_[index]->SetIsAdded_(true);
			}
			
		}
		if (controller_[index]->GetIsActived_()) 
		{
			controller_[index]->LimitPose(pose);
			controller_[index]->UpdatePose(pose, dp_hmd_->GetIsActived_());
			if (streaming_hmd_->GetIsActived_())
			{
				controller_[index]->UpdateButtonState(*pose);
			}
			else if (dp_hmd_->GetIsActived_())
			{
				controller_[index]->UpdateButtonState(button_state);
			}
		}
	

	}
}

DWORD WINAPI PicoVRDriver::EventPollThread(PVOID param)
{
    PicoVRDriver* pStreamingHmdDriver = (PicoVRDriver*)(param);

    pStreamingHmdDriver->EventPollFunc();

    return 0;
}
struct NotifySendMode 
{
	int BeginFlag;
	int Type;
	int EndFlag;
};
#define NotifySendBeginFlag 2222;
#define NotifySendEndFlag 22222;
void PicoVRDriver::EventPollFunc()
{
    vr::VREvent_t vr_event;
	int sendindex = 0;
    while (!m_bExitEventThread) {
		while (vr::VRServerDriverHost()->PollNextEvent(&vr_event, sizeof(vr_event))) {
			if (vr_event.eventType == VREvent_Input_HapticVibration) {
				RVR_LOG_I("Vibration event (handle %llu, amplitude %f, duration %0.6f, frequency %0.6f)",
					vr_event.data.hapticVibration.componentHandle,
					vr_event.data.hapticVibration.fAmplitude,
					vr_event.data.hapticVibration.fDurationSeconds,
					vr_event.data.hapticVibration.fFrequency);

				RVREventVibration info;
				info.eventType = (uint32_t)RVREvent::RVR_EVENT_VIBRATION;
				info.controller_index = -1;
				info.amplitude = vr_event.data.hapticVibration.fAmplitude;
				info.durationSeconds = vr_event.data.hapticVibration.fDurationSeconds;
				info.frequency = vr_event.data.hapticVibration.fFrequency;

				if (info.frequency < 5)
				{
					info.frequency = 5;
				}
				if (gConfigReader.GetHmdType().compare("neo2") == 0)
				{
					if (info.durationSeconds < 0.1)
					{
						info.durationSeconds = 0.1;
					}
				}
				//else if ((gConfigReader.GetHmdType().compare("neo3") == 0))
				{
					if (info.durationSeconds < gConfigReader.GetVibrationtime())
					{
						info.durationSeconds = gConfigReader.GetVibrationtime();
					}
				}
				for (int i = 0; i < 2; i++) {
					if (controller_[i]->GetHapticsHandle() == vr_event.data.hapticVibration.componentHandle) {
						info.controller_index = i;
						break;
					}
				}
				if (info.controller_index != -1) {
					if (controller_[info.controller_index]->GetPose().deviceIsConnected==false)
					{
						continue;
					}
					if (streaming_hmd_->GetIsActived_())
					{
						char sendmsg[1024] = { 0 };
						int len = 0;
						sendmsg[0] = 0x12;
						sendmsg[1] = 0x14;
						memmove(sendmsg + 2, &info, sizeof(RVREventVibration));
						sendmsg[2 + sizeof(RVREventVibration)] = 0xee;
						len = 2 + sizeof(RVREventVibration) + 1;
						//SensorSocket::GetInstance(-1)->SendMsg(sendmsg, len);
						if (gConfigReader.GetRtcOrBulkMode_() == 2)
						{
                            auto usb_bulk_module = pico_streaming::UsbBulkModule::GetInstance();
                            if (usb_bulk_module->GetEngineStartup())
                            {
                                usb_bulk_module->SendMsg(sendmsg, len);
                            }
						}
						else
						{
							SensorSocket::GetInstance(-1)->SendMsg(sendmsg, len);
						}
						//send haptics message
					} 
					else if(dp_hmd_->GetIsActived_())
					{
						HidType::HapticVibration haptic_vibration;
						haptic_vibration.amplitude = vr_event.data.hapticVibration.fAmplitude;
						haptic_vibration.frequency = vr_event.data.hapticVibration.fFrequency;
						haptic_vibration.duration_seconds = vr_event.data.hapticVibration.fDurationSeconds;
						haptic_vibration.type = info.controller_index;
						haptic_vibration.data_type = 1;
						if (haptic_vibration.frequency < 1)
						{
							haptic_vibration.frequency = 1;
						}

						if (haptic_vibration.duration_seconds < 0.1)
						{
							haptic_vibration.duration_seconds = 0.1;
						}

						int retval = HidModule::GetInstance()->WriteData((unsigned char*)&haptic_vibration, sizeof(HidType::HapticVibration));
						

					}
					
				}
			}
			else 
			{
 				//DriverLog("vr_envent %d", vr_event.eventType);
				 
				if (vr_event.eventType == vr::VREvent_DashboardActivated)
				{
					gDashboardActivated = true;
				}
				if (vr_event.eventType == vr::VREvent_DashboardDeactivated)
				{
					gDashboardActivated = false;
				}

				if (vr_event.eventType == vr::VREvent_OverlayShown)
				{
					gOverlayShow= true;
				}
				if (vr_event.eventType == vr::VREvent_OverlayHidden)
				{
					gOverlayShow = false;
				}

				if (vr_event.eventType == vr::VREvent_ProcessQuit)
				{
					gBasePid=0;
				}
				 
			}
		}
		
		//g_svrDriver.GetController(0)->UpdateButtonState(g_svrDriver.GetController(0)->last_data);
		//g_svrDriver.GetController(1)->UpdateButtonState(g_svrDriver.GetController(1)->last_data);
        Sleep(3);
    }
}

DWORD WINAPI  PicoVRDriver::PicoPanelProtecterThread(PVOID param)
{
	PicoVRDriver* pStreamingHmdDriver = (PicoVRDriver*)(param);

	pStreamingHmdDriver->PicoPanelProtecterFunc();
	return 0;
}
void PicoVRDriver::PicoPanelProtecterFunc() 
{
	bool last_test_sensor = false;
	while (!m_bExitEventThread) 
	{
		if (last_test_sensor==false)
		{
			if (g_test_sensor==true)
			{
				_beginthreadex(NULL, 0, &TestSensorThread, this, 0, 0);
			}
			
		}
		last_test_sensor = g_test_sensor;
		if (streaming_hmd_->GetIsActived_()||dp_hmd_->GetIsActived_())
		{
			if (isProgramRunning("pico_panel.exe") == false)
			{
				Sleep(2000);
				std::string pico_panel = driver_folder_;
				pico_panel = pico_panel +"pico_panel\\pico_panel.exe";
				if (_access(pico_panel.c_str(), 0) != -1)
				{
					LPCSTR buf[256] = { 0 };
					strcpy((char*)buf, driver_folder_.c_str());
					HINSTANCE hNewExe = ShellExecuteA(NULL, "open", pico_panel.c_str(),(LPCSTR) buf, (LPCSTR)buf, SW_HIDE);
					if ((DWORD)hNewExe <= 32)
					{
						OutputDebugStringW(L"pico_panel.exe run failed");
					}
				}
			}

			if (isProgramRunning("app_check.exe") == false)
			{
				 
				std::string app_check = driver_folder_;
				app_check = app_check + "app_check.exe";
				if (_access(app_check.c_str(), 0) != -1)
				{
					LPCSTR buf[256] = { 0 };
					strcpy((char*)buf, driver_folder_.c_str());
					HINSTANCE hNewExe = ShellExecuteA(NULL, "open", app_check.c_str(), (LPCSTR)buf, (LPCSTR)buf, SW_HIDE);
					if ((DWORD)hNewExe <= 32)
					{
						OutputDebugStringW(L"app_check.exe run failed");
					}
				}
			}
		}
		
		Sleep(200);
	}
	
}