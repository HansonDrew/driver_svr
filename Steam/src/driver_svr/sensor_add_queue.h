#pragma once
#include<mutex>
#include <queue>
#include "../RVRPlugin/RVRPluginDefinitions.h"
 
class sensor_add_queue
{
public:
	std::queue<RVR::RVRPoseHmdData>hmd_pose_queue_;
	std::queue<RVR::RVRControllerData> controller_pose_queue_[2];
	std::mutex pose_lock_;

	void SetPose(RVR::RVRPoseHmdData& hmd_pose, RVR::RVRControllerData& left_controller_pose, RVR::RVRControllerData& right_controller_pose);
	void GetPose(RVR::RVRPoseHmdData& hmd_pose, RVR::RVRControllerData& left_controller_pose, RVR::RVRControllerData& right_controller_pose);
	void GetPoseWithTimestamp(RVR::RVRPoseHmdData& hmd_pose, RVR::RVRControllerData& left_controller_pose, RVR::RVRControllerData& right_controller_pose,int64_t last_present_ts);
	int GetPoseWithTimeStore( RVR::RVRPoseHmdData& hmd_pose, RVR::RVRControllerData& left_controller_pose, RVR::RVRControllerData& right_controller_pose);
	void GetPoseWithSmooth(RVR::RVRPoseHmdData& hmd_pose, RVR::RVRControllerData& left_controller_pose, RVR::RVRControllerData& right_controller_pose);
private:

	
	void SmoothSensor();
	int64_t new_receive_sdk_ts_ = 0;
	int64_t new_receive_ts_ = 0;
	int64_t last_use_sdk_ts_ = 0;
	int64_t last_present_ts_ = 0;
	int64_t last_get_time_ = 0;
	RVR::RVRPoseHmdData last_hmd_pose_ = { 0 };
	RVR::RVRControllerData last_left_controller_pose_ = { 0 };
	RVR::RVRControllerData last_right_controller_pose_ = { 0 };
	void GetPredictHmdPose(RVR::RVRPoseHmdData& hmd_pose);
	void GetPredictControllerPose(RVR::RVRControllerData& controller_pose,int controller_index);
};

