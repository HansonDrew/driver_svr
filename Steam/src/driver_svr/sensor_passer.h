#pragma once
#include <mutex>
#include "../RVRPlugin/RVRPluginDefinitions.h"
#include "driver_define.h"
#include<windows.h>
#include "ControllerTracker.h"
class SensorPasser
{
public:
	static SensorPasser* GetInstance();
	HANDLE new_sensor_notify_;
	void SetHmdSensor(RVR::RVRPoseHmdData hmd_pose_t);
	void SetControllerSensor(RVR::RVRControllerData controller_pose_t, ControllerIndex controller_index);
	void SetAllSensor(RVR::RVRPoseHmdData hmd_pose_t, RVR::RVRControllerData left_controller_pose, RVR::RVRControllerData right_controller_pose);
	void GetHmdSensor(RVR::RVRPoseHmdData &hmd_pose_t);
	void GetControllerSensor(RVR::RVRControllerData &controller_pose_t, ControllerIndex controller_index);
	void SetNewSensorNotifyEvent();
	void AddSensorByNotifyEvent(int64_t present_time,int wait_time=0xFFFFFFFF);
	void AddSensorByNotifyEventWithWait(int64_t present_time, int wait_time = 0xFFFFFFFF);

	void AddSensorByNotifyEventWithSmooth(int64_t present_sub_time);
	bool AddSensorByTimestamp(int64_t last_present_time);

	void AddNewSensorWithPredict(int64_t present_sub_time);
	void AddNewSensorWithPredictAndKalmanFilter(int64_t present_sub_time);
	void AddSensorWithTimeStore( );
	void ResetNewSensorNotyfyEvent();
	bool GetNetBadMode_() { return net_bad_mode_; };
	void SetNetBadMode_(int net_bad) { net_bad_mode_ = net_bad; };
	ControllerTracker LeftKalman;
	ControllerTracker RightKalman;
	ControllerTracker HmdKalman;
	int per_frame = 0;
	
	int64_t last_sdk_time_ = -1;
	int64_t last_get_sdk_time_ = -1;
	int64_t last_present_time_ = -1;
	int64_t last_sensor_get_time_ = -1;
	int64_t last_sensor_recv_time_ = -1;
	class GC
	{
	public:
		GC()
		{
		}
		~GC()
		{

			if (sensor_passer_instance_ != nullptr)
			{
				delete sensor_passer_instance_;
				sensor_passer_instance_ = nullptr;
			}

		}
	};
	static GC sensor_passer_gc_;
protected:
	SensorPasser();
	~SensorPasser();

private:
	bool net_bad_mode_ = false;
	std::mutex all_mutex_;
	std::mutex hmd_mutex_;
	std::mutex controller_mutex_[2];
	RVR::RVRPoseHmdData hmd_pose_ = { 0 };
	RVR::RVRControllerData controller_pose_[2] = { 0 };
	static SensorPasser* sensor_passer_instance_;
private:
};

