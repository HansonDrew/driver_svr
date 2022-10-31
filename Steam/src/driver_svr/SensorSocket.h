#pragma once
#include <stdio.h>
#include <winsock2.h>
#include <stdint.h>
#include <functional>
#include "driver_define.h"
#include "../RVRPluginDefinitions.h"
#define SENSORBUFLEN 59
#define NEWSENSORBUFLEN 107
#define CONTROLLERSENSORBUFLEN 168+3

typedef std::function<void( void* data )> SetRvrPose;

class SensorSocket
{
protected:
	SensorSocket() {};
	~SensorSocket();
public:
	
	RVR::RVRPoseHmdData hmd_pose_ = { 0 };
	RVR::RVRControllerData left_pose_ = { 0 };
	RVR::RVRControllerData right_pose_ = { 0 };
	int64_t last_recv_time_ = -1;
	int64_t last_sdk_time_ = -1;

	int64_t base_sdk_time_ = -1;
	SetRvrPose mRvrPoseCallBack;
	int64_t last_sensor_index_ = 0;
	static unsigned int __stdcall  RecvThread(LPVOID lpParameter);
	bool loop_;
	void ResetTimeStamp()
	{
		last_recv_time_ = last_sdk_time_ = last_sensor_index_ = 0;
		base_sdk_time_ = 0;
	};
	WireLessType::TransPoseData mTransPose;
	void GetPose(WireLessType::TransPoseData &pose);
	void SetPose(void *data, RVR::RVRPoseHmdData  &hmd_pose);
	void SetControllerPose(void* data, RVR::RVRControllerData& controller_pose);
	void SetCallBack(SetRvrPose callback) 
	{
		mRvrPoseCallBack = callback;
	};
	static SensorSocket *GetInstance(int index = -1);
	int mIndex;
	int InitSocket(char* ip, u_short port );
	int ShutDown();
	int SendMsg(char *buf,int len);
	int SendMsg(char* buf, int len, char* ip, u_short port);
	uint64_t last_time_stamp=0;
	class GC // 垃圾回收类
	{
	public:
		GC()
		{
		}
		~GC()
		{
			// We can destory all the resouce here, eg:db connector, file handle and so on
			
				if (gSensorSocketInstance != NULL)
				{
					delete gSensorSocketInstance;
					gSensorSocketInstance = NULL;
				}
			
			 
		}
	};
	static GC SensorSocketgc;  //垃圾回收类的静态成员
	SOCKET mServerSocket;
	SOCKET mhelpSocket;
	sockaddr_in m_ClientSocketaddr;
private:
	static SensorSocket *gSensorSocketInstance;
	static SensorSocket *LeftControllerInstance;
	static SensorSocket *RightControllerInstance;
};

class ControllerSensorSocket:public SensorSocket
{
	friend class SensorSocket;
private:
	ControllerSensorSocket() { last_time_stamp = 0; };
	~ControllerSensorSocket() ;
public:
	RVR::RVRControllerData controller_pose_;
	SetRvrPose mRvrPoseCallBack;
	static unsigned int __stdcall  RecvThread(LPVOID lpParameter);
	WireLessType::TransControllerData mTransControllerPose;
	void GetPose(WireLessType::TransControllerData &pose);
	void SetPose(void *data,  RVR::RVRControllerData& controller_sensor);
	uint64_t last_time_stamp;
private:
	
};
