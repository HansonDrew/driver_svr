#include<memory.h>
#include <string>
#include "SensorManger.h"
#include "driver_define.h"
#include "RVRLogger.h"
#include "RVRPluginDefinitions.h"
#include "driverlog.h"
using namespace RVR;
SensorManger* SensorManger::sensor_manger_ =NULL;
SensorManger* SensorManger::GetInstance()
{
	if (sensor_manger_==NULL)
	{
		sensor_manger_ = new SensorManger;
	}
	return sensor_manger_;
}
void SensorManger::GetSensor(PictureControlType control_type, int eye_index, RVR::RVRPoseHmdData& out_sensor) 
{
	 
		RVR::RVRPoseHmdData frame_sensor= sensor_buf_[send_inedx_ % SensorBufSize];
		out_sensor = sensor_buf_[send_inedx_ % SensorBufSize];
	 
	 
	//DriverLog("GetSensor index=%llu r=%f,%f,%f,%f p=%f,%f,%f,",
	//	out_sensor.poseTimeStamp, out_sensor.rotation.w, out_sensor.rotation.x,
	//	out_sensor.rotation.y, out_sensor.rotation.z,
	//	out_sensor.position.x, out_sensor.position.y, out_sensor.position.z);
  
}

void SensorManger::SaveSensor(RVR::RVRPoseHmdData sensor) 
{
	sensor.poseTimeStamp = save_index_;
	sensor_buf_[save_index_ % SensorBufSize] = sensor;
	//DriverLog("SaveSensor index= %llu",save_index_);
	//DriverLog("SaveSensor index=%llu r=%f,%f,%f,%f p=%f,%f,%f,",
	//	sensor.poseTimeStamp, sensor.rotation.w, sensor.rotation.x,
	//	sensor.rotation.y, sensor.rotation.z,
	//	sensor.position.x, sensor.position.y, sensor.position.z);
	save_index_++;
}

bool SensorManger::ConvertToHmdSensor(uint8_t* buf, int length, RVR::RVRPoseHmdData& hmd_pose)
{
	WireLessType::TransPoseData* pose = (WireLessType::TransPoseData*)buf;
	
	hmd_pose.valid = true;
	hmd_pose.position.x = pose->position.x;
	hmd_pose.position.y = pose->position.y;
	hmd_pose.position.z = pose->position.z;

	hmd_pose.rotation.x = pose->rotation.x;
	hmd_pose.rotation.y = pose->rotation.y;
	hmd_pose.rotation.z = pose->rotation.z;
	hmd_pose.rotation.w = pose->rotation.w;

	hmd_pose.poseRecvTime = RVR::nowInNs();
	hmd_pose.poseTimeStamp = pose->poseTimeStamp;
	hmd_pose.predictedTimeMs = pose->predictedTimeMs;

	hmd_pose.position.x = pose->position.x;
	hmd_pose.position.y = pose->position.y;
	hmd_pose.position.z = pose->position.z;

	hmd_pose.linearVelocity.x = pose->linearVelocity.x;
	hmd_pose.linearVelocity.y = pose->linearVelocity.y;
	hmd_pose.linearVelocity.z = pose->linearVelocity.z;

	hmd_pose.linearAcceleration.x = pose->linearAcceleration.x;
	hmd_pose.linearAcceleration.y = pose->linearAcceleration.y;
	hmd_pose.linearAcceleration.z = pose->linearAcceleration.z;

	hmd_pose.angularVelocity.x = pose->angularVelocity.x;
	hmd_pose.angularVelocity.y = pose->angularVelocity.y;
	hmd_pose.angularVelocity.z = pose->angularVelocity.z;

	hmd_pose.angularAcceleration.x = pose->angularAcceleration.x;
	hmd_pose.angularAcceleration.y = pose->angularAcceleration.y;
	hmd_pose.angularAcceleration.z = pose->angularAcceleration.z;
	return true;
	 
}
bool SensorManger::ConvertToControllerSensor(uint8_t* buf, int length, int controller_index, RVR::RVRControllerData& controller_pose)
{
	WireLessType::TransControllerData* pose = (WireLessType::TransControllerData*)buf;

	memmove(controller_pose.analog1D, pose->analog1D, sizeof(float) * 8);
	memmove(controller_pose.analog2D, pose->analog2D, sizeof(RVR::RVRVector2) * 4);
	controller_pose.buttonState = pose->buttonState;
	controller_pose.connectionState = (RVR::RVRControllerConnectionState)pose->connectionState;
	controller_pose.isTouching = pose->isTouching;
	controller_pose.position.x = pose->position.x;
	controller_pose.position.y = pose->position.y;
	controller_pose.position.z = pose->position.z;
	controller_pose.rotation.x = pose->rotation.x;
	controller_pose.rotation.y = pose->rotation.y;
	controller_pose.rotation.z = pose->rotation.z;
	controller_pose.rotation.w = pose->rotation.w;

	controller_pose.vecAcceleration.x = pose->vecAcceleration.x;
	controller_pose.vecAcceleration.y = pose->vecAcceleration.y;
	controller_pose.vecAcceleration.z = pose->vecAcceleration.z;
	controller_pose.vecAngularAcceleration.x = pose->vecAngularAcceleration.x;
	controller_pose.vecAngularAcceleration.y = pose->vecAngularAcceleration.y;
	controller_pose.vecAngularAcceleration.z = pose->vecAngularAcceleration.z;
	controller_pose.vecAngularVelocity.x = pose->vecAngularVelocity.x;
	controller_pose.vecAngularVelocity.y = pose->vecAngularVelocity.y;
	controller_pose.vecAngularVelocity.z = pose->vecAngularVelocity.z;
	controller_pose.vecVelocity.x = pose->vecVelocity.x;
	controller_pose.vecVelocity.y = pose->vecVelocity.y;
	controller_pose.vecVelocity.z = pose->vecVelocity.z;
	return true;
}

void SensorManger::SetIndex(int index)
{
	send_inedx_ = index;
}