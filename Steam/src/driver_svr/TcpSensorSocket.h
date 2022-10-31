#pragma once
#include<WinSock2.h>
#include<string>

#include "../RVRPluginDefinitions.h"
class TcpSensorSocket
{
public:

	static TcpSensorSocket* GetInstance();
	static TcpSensorSocket* CreateInstance();

	class GC // 垃圾回收类
	{
	public:
		GC()
		{
		}
		~GC()
		{
			// We can destory all the resouce here, eg:db connector, file handle and so on
			if (gTcpSensorInstance != NULL)
			{
				delete gTcpSensorInstance;
				gTcpSensorInstance = NULL;
			}
			 
		}
	};

	RVR::RVRPoseHmdData hmd_pose_ = { 0 };
	RVR::RVRControllerData left_pose_ = { 0 };
	RVR::RVRControllerData right_pose_ = { 0 };
	


	static GC  TcpSensoorGc;  //垃圾回收类的静态成员
	
	SOCKET locate_socket_;
	SOCKET hmd_socket_;
	SOCKADDR_IN hmd_addr_;
	HANDLE recv_thread_event_ = INVALID_HANDLE_VALUE;
	HANDLE check_thread_event_ = INVALID_HANDLE_VALUE;
	void SetHmdPose(void* data, RVR::RVRPoseHmdData& hmd_pose);
	void SetControllerPose(void* data, RVR::RVRControllerData& controller_pose,int left_or_right);
	static unsigned int __stdcall WorkThread(LPVOID lpParameter);
	static unsigned int __stdcall CheckSensorThread(LPVOID lpParameter);
	int InitSocket(std::string ip, u_short port);
	int CloseSocket();
	int CloseHmdSocket();
	int GetSensorFromSocket(char *buf,int len);
	FILE* pf = NULL;
#define  TcpSensorBufLen  1024*1024
	char recv_buf_[TcpSensorBufLen];
	int recv_len_=0;
	unsigned mReconnectThreadId;

	std::string mIp;
	int mPort;

	unsigned short m_usSeqNum;
	~TcpSensorSocket() {   };
	uint64_t mConnectTime = 0;
	u_short socket_port;
	bool GetLoop() { return loop_; };
	int64_t last_hmd_sdk_ts_ = -1;
	int64_t last_left_sdk_ts_ = -1;
	int64_t last_right_sdk_ts_ = -1;
	int64_t last_recv_time_ = -1;
private:
	int init_flag_ = false;
	static TcpSensorSocket* gTcpSensorInstance;
	bool loop_ = false;
	std::string dst_ip_;
	u_short dst_port_;
};

