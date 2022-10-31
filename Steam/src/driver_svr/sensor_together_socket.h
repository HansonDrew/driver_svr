#pragma once


#pragma once
#include <stdio.h>
#include <winsock2.h>
#include <stdint.h>
#include <functional>
#include "driver_define.h"
#include "../RVRPluginDefinitions.h"
 
#define TOGETHERSENSORBUFLEN 107+168*2
 
 

class sensor_together_socket
{
protected:
	sensor_together_socket() {};
	~sensor_together_socket();
public:

	RVR::RVRPoseHmdData hmd_pose_ = { 0 };
	RVR::RVRControllerData left_pose_ = { 0 };
	RVR::RVRControllerData right_pose_ = { 0 };
	int64_t last_recv_time_ = -1;
	int64_t last_sdk_time_ = -1;
	int64_t base_sdk_time_ = -1;
	void ResetRVRControllerData(RVR::RVRControllerData& pose_data);
	int64_t last_sensor_index_ = -1;
	static unsigned int __stdcall  RecvThread(LPVOID lpParameter);
	bool RecvLoop;
	void ResetTimeStamp()
	{
		last_recv_time_ = last_sdk_time_ = last_sensor_index_ = 0;
		base_sdk_time_ = 0;
	};
	
	
	void SetHmdPose(void* data, RVR::RVRPoseHmdData& hmd_pose,bool realy_update=true);
	
	void SetControllerPose(void* data, RVR::RVRControllerData& controller_pose);
	static sensor_together_socket* GetInstance();
	int mIndex;
	int InitSocket(char* ip, u_short port);

	int SendMsg(char* buf, int len);
	int SendMsg(char* buf, int len, char* ip, u_short port);
	uint64_t last_time_stamp = 0;
	class GC // 垃圾回收类
	{
	public:
		GC()
		{
		}
		~GC()
		{
			// We can destory all the resouce here, eg:db connector, file handle and so on

			if (g_sensor_together_socket_ != NULL)
			{
				delete g_sensor_together_socket_;
				g_sensor_together_socket_ = NULL;
			}


		}
	};
	static GC SensorSocketgc;  //垃圾回收类的静态成员
	SOCKET mServerSocket;
	SOCKET mhelpSocket;
	sockaddr_in m_ClientSocketaddr;
private:
	static sensor_together_socket* g_sensor_together_socket_;
	 
};
