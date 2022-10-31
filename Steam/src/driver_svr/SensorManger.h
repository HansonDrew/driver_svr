#pragma once
#include "../RVRPluginDefinitions.h"
enum class PictureControlType
{
	KLittlePicture=0,
    KBigPicture=1 
};
#define SensorBufSize 100
class SensorManger
{
public:
	 
	static SensorManger* GetInstance();
	void SaveSensor(RVR::RVRPoseHmdData sensor);
	 
	void GetSensor(PictureControlType control_type,int eye_index, RVR::RVRPoseHmdData&out_sensor);
	bool ConvertToHmdSensor(uint8_t*buf,int length, RVR::RVRPoseHmdData& hmd_pose);
	bool ConvertToControllerSensor(uint8_t* buf, int length,int controller_index, RVR::RVRControllerData& controller_pose);
	void SetIndex(int index);
private:
	RVR::RVRPoseHmdData sensor_buf_[SensorBufSize];
	uint64_t save_index_ = 0 ;
	int controller_type_ = 0;
	uint64_t left_inedx_=0;
	uint64_t right_index_ = 0;
	static SensorManger* sensor_manger_ ;
	uint64_t last_hmd_timestamp;
	uint64_t last_left_timestamp;
	uint64_t last_right_timestamp;
};

