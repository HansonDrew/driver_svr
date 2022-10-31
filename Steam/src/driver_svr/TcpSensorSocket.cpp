#include "TcpSensorSocket.h"
#include "RVRLogger.h"
#include "driver_define.h"
#include "sensor_passer.h"
#include "driver_pico.h"
using namespace RVR;
#include "config_reader.h"
extern ConfigReader gConfigReader;

extern PicoVRDriver g_svrDriver;
extern std::string 	gDstip;
extern bool  setip;
TcpSensorSocket* TcpSensorSocket::gTcpSensorInstance = nullptr;

TcpSensorSocket::GC TcpSensorSocket::TcpSensoorGc;

TcpSensorSocket* TcpSensorSocket::GetInstance()
{

	if (gTcpSensorInstance == NULL)
	{
		return CreateInstance();
	}
	return gTcpSensorInstance;
	
	return nullptr;
}

TcpSensorSocket* TcpSensorSocket::CreateInstance()
{
	 
	gTcpSensorInstance = new TcpSensorSocket();
 
	return nullptr;
}


int  TcpSensorSocket::InitSocket(std::string ip, u_short port) 
{
	
	
	check_thread_event_=CreateEvent(NULL, FALSE, FALSE, NULL);
	recv_thread_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	loop_ = true;
	HANDLE ret = (HANDLE)_beginthreadex(NULL, 0, &WorkThread, this, 0,NULL);
	SetThreadPriority(ret, THREAD_PRIORITY_TIME_CRITICAL);
	if (ret != NULL)
	{
		CloseHandle(ret);
	}
	_beginthreadex(NULL, 0, &CheckSensorThread, this, 0, NULL);
	locate_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct linger so_linger;
	so_linger.l_onoff = 0;
	so_linger.l_linger = 0;
	setsockopt(locate_socket_, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof so_linger);

	int i = 1; setsockopt(locate_socket_, IPPROTO_TCP, TCP_NODELAY, (char*)&i, sizeof(i));

	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	if (SOCKET_ERROR == bind(locate_socket_, (sockaddr*)&_sin, sizeof(_sin)))
	{
		RVR_LOG_A("ERROR，sensor tcp bind error");
	}
	else {
		RVR_LOG_A("ensor tcp bind ok");
	}

	if (SOCKET_ERROR == listen(locate_socket_, 32))
	{
		RVR_LOG_A("ERROR，sensor tcp listen error");
	}
	else {
		RVR_LOG_A("sensor tcp listen ok");
	}



	RVR_LOG_A("sensor tcp listen init ok");
	return 1;
}
int  TcpSensorSocket::CloseSocket() 
{
	loop_ = false;
	DriverLog(" tcp CloseSocket ");
	closesocket(hmd_socket_);
	closesocket(locate_socket_);
	WaitForSingleObject(check_thread_event_, INFINITE);
	WaitForSingleObject(recv_thread_event_, INFINITE);
	DriverLog("sensor tcp CloseSocket ok");
	return 1;
}
int TcpSensorSocket::CloseHmdSocket() 
{
	closesocket(hmd_socket_);
	recv_len_ = 0;
	memset(recv_buf_, 0, sizeof(char)* TcpSensorBufLen);
	return 1;
}
int TcpSensorSocket::GetSensorFromSocket(char* buf, int len)
{
	//RVR_LOG_A("socket get tcp sensor");
	int offset = 0;
	SetHmdPose(buf + offset, hmd_pose_);
	offset += sizeof(WireLessType::TransPoseData);
	SetControllerPose(buf + offset, left_pose_, 0);
	offset += sizeof(WireLessType::TransControllerData);
	SetControllerPose(buf + offset, right_pose_, 1);
	
	SensorPasser::GetInstance()->SetAllSensor(hmd_pose_, left_pose_, right_pose_);
	if (g_svrDriver.GetStreamingHmdDriver()->GetIsAdded_() == false)
	{
	
		g_svrDriver.AddControllerPose((uint32_t)ControllerIndex::kLeftController, &TcpSensorSocket::GetInstance()->left_pose_);
		g_svrDriver.AddControllerPose((uint32_t)ControllerIndex::kRightController, &TcpSensorSocket::GetInstance()->right_pose_);
		g_svrDriver.AddHmdPose(&TcpSensorSocket::GetInstance()->hmd_pose_, false);
	}

	/*RVR_LOG_A("sensertest:%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%lld",
		hmd_pose_.rotation.w, hmd_pose_.rotation.x, hmd_pose_.rotation.y, hmd_pose_.rotation.z, hmd_pose_.position.x, hmd_pose_.position.y, hmd_pose_.position.z,
		left_pose_.rotation.w, left_pose_.rotation.x, left_pose_.rotation.y, left_pose_.rotation.z, left_pose_.position.x, left_pose_.position.y, left_pose_.position.z,
		right_pose_.rotation.w, right_pose_.rotation.x, right_pose_.rotation.y, right_pose_.rotation.z, right_pose_.position.x, right_pose_.position.y, right_pose_.position.z,
		hmd_pose_.poseTimeStamp);
*/

	// present 中都不sleep
	/// 不用队列。每次都刷新 sensor 。直接放给present 调度  
	// 不用队列。每次都刷新 sensor 。 用 present 调度 加预测 
	//  recv中直接 addpose 。 add 时 加 predict 
	// 用队列····
	// 1 present no sleep  2 present by sensor 3 predict ? 4 another thread delete from queue by ts??
	return 1;
}
unsigned int TcpSensorSocket::CheckSensorThread(LPVOID lpParameter) 
{
	ResetEvent(TcpSensorSocket::GetInstance()->check_thread_event_);
	while (TcpSensorSocket::GetInstance()->GetLoop())
	{
		if (TcpSensorSocket::GetInstance()->last_recv_time_<=0)
		{
			Sleep(1);
			continue;
		}
		
		int64_t ts = nowInNs();
		//DriverLog("check sensor ts=%lld last=%lld ts-last =%lld\n", ts, TcpSensorSocket::GetInstance()->last_recv_time_, ts - TcpSensorSocket::GetInstance()->last_recv_time_);
		if ((ts- TcpSensorSocket::GetInstance()->last_recv_time_)>2000000000)
		{
			RVR::RVRPoseHmdData hmd_pose = { 0 };
			memmove(&hmd_pose, &TcpSensorSocket::GetInstance()->hmd_pose_, sizeof(RVR::RVRPoseHmdData));
			hmd_pose.valid = FALSE;

			RVR::RVRControllerData controller_pose[2] = { 0 };
			
			memmove(&controller_pose[0], &TcpSensorSocket::GetInstance()->left_pose_, sizeof(RVR::RVRPoseHmdData));
			controller_pose[0].connectionState = RVR::RVRControllerConnectionState::kDisconnected;

			memmove(&controller_pose[1], &TcpSensorSocket::GetInstance()->right_pose_, sizeof(RVR::RVRPoseHmdData));
			controller_pose[1].connectionState = RVR::RVRControllerConnectionState::kDisconnected;

			if (gConfigReader.GetRtcOrBulkMode_() == 0)
			{
				g_svrDriver.AddHmdPose(&hmd_pose);
				g_svrDriver.AddControllerPose(0, &controller_pose[0]);
				g_svrDriver.AddControllerPose(1, &controller_pose[1]);
			}

		}
		if ((ts - TcpSensorSocket::GetInstance()->last_recv_time_) > 2000000000) 
		{
			if ((ts - TcpSensorSocket::GetInstance()->last_recv_time_) > 20000000000) 
			{
				TcpSensorSocket::GetInstance()->CloseHmdSocket();
			}
		}
		Sleep(500);
	}
	SetEvent(TcpSensorSocket::GetInstance()->check_thread_event_);
	DriverLog("sensor tcp CheckSensorThread quit");
	return 1;
}
unsigned int  TcpSensorSocket::WorkThread(LPVOID lpParameter) 
{
	RVR_LOG_A("sensor tcp WorkThread ok");
	ResetEvent(TcpSensorSocket::GetInstance()->recv_thread_event_);
	while (TcpSensorSocket::GetInstance()->GetLoop())
	{
		int len = sizeof(SOCKADDR);
		DriverLog("sensor keep listen");
		TcpSensorSocket::GetInstance() ->hmd_socket_= accept(TcpSensorSocket::GetInstance() ->locate_socket_, (SOCKADDR*)&TcpSensorSocket::GetInstance()->hmd_addr_, &len);
		struct linger so_linger;
		so_linger.l_onoff = 0;
		so_linger.l_linger = 0;
		setsockopt(TcpSensorSocket::GetInstance()->hmd_socket_, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof so_linger);
		DriverLog("sensor tcp client connected");
		gDstip = inet_ntoa(TcpSensorSocket::GetInstance()->hmd_addr_.sin_addr);
		setip = true;
		DriverLog("get hmd tcpsensor connected");
		int64_t socket_ts = 0;
		int loopfirst = 0;
		while (TcpSensorSocket::GetInstance()->GetLoop())
		{
			if (loopfirst==0)
			{
				char msg[256] = "picovr go";
				send(TcpSensorSocket::GetInstance()->hmd_socket_,msg,256,0);
				loopfirst++;
			}
			
			char recv_buf[SensorTcpBufLen];
		 
			int recv_len=recv(TcpSensorSocket::GetInstance()->hmd_socket_, recv_buf, SensorTcpBufLen, 0);
			if (recv_len<=0)
			{
				TcpSensorSocket::GetInstance()->CloseHmdSocket();
				break;
			}
		
			memmove(TcpSensorSocket::GetInstance()->recv_buf_ + TcpSensorSocket::GetInstance()->recv_len_, recv_buf, recv_len);
			TcpSensorSocket::GetInstance()->recv_len_ = TcpSensorSocket::GetInstance()->recv_len_ + recv_len;

			while (TcpSensorSocket::GetInstance()->recv_len_ >=SensorTcpBufLen)
			{
				char msg_type = TcpSensorSocket::GetInstance()->recv_buf_[0];
				if (msg_type==0x11)
				{
					int msg_len = 0;
					memmove(&msg_len, TcpSensorSocket::GetInstance()->recv_buf_ + 1, sizeof(int));
					if (msg_len== TCPSENSORMSGLEN)
					{
						int64_t ts = RVR::nowInNs();
					//	RVR_LOG_A("socket subts=%lld", (ts - socket_ts) / 1000);
						socket_ts = ts;
						if ((TcpSensorSocket::GetInstance()->recv_len_ - SensorTcpBufLen)<  SensorTcpBufLen)
						{
							TcpSensorSocket::GetInstance()->GetSensorFromSocket(TcpSensorSocket::GetInstance()->recv_buf_ + 5, TCPSENSORMSGLEN);
						}
						else
						{
							//RVR_LOG_A("not add  subts=%lld", (ts - socket_ts) / 1000);
						}
						TcpSensorSocket::GetInstance()->recv_len_ -= SensorTcpBufLen;
						if (TcpSensorSocket::GetInstance()->recv_len_>0)
						{
							memmove(TcpSensorSocket::GetInstance()->recv_buf_, TcpSensorSocket::GetInstance()->recv_buf_ + SensorTcpBufLen, TcpSensorSocket::GetInstance()->recv_len_);
						}
					
					}
					else
					{
						RVR_LOG_A("msg sensor length error,length= %d",msg_len);
						TcpSensorSocket::GetInstance()->CloseHmdSocket();
					}
				}
				else
				{
					RVR_LOG_A("recv msg type error");
					TcpSensorSocket::GetInstance()->CloseHmdSocket();
				}
			}

		}

	}
	DriverLog("sensor tcp thread quit ok");
	SetEvent(TcpSensorSocket::GetInstance()->recv_thread_event_);
	return 1;
}


void TcpSensorSocket::SetControllerPose(void* data, RVR::RVRControllerData& controller_pose, int left_or_right)
{
	WireLessType::TransControllerData* p_pose = (WireLessType::TransControllerData*)data;

	WireLessType::TransControllerData TransControllerPose = *p_pose;

	controller_pose = { 0 };

	memmove(&controller_pose.analog1D, &TransControllerPose.analog1D, sizeof(float) * 8);
 
 
	memmove(&controller_pose.analog2D, &TransControllerPose.analog2D, sizeof(RVRVector2) * 4);
	controller_pose.buttonState = TransControllerPose.buttonState;
	controller_pose.connectionState = (RVR::RVRControllerConnectionState)TransControllerPose.connectionState;
	controller_pose.isTouching = TransControllerPose.isTouching;
	controller_pose.position.x = TransControllerPose.position.x;
	controller_pose.position.y = TransControllerPose.position.y;
	controller_pose.position.z = TransControllerPose.position.z;
	controller_pose.rotation.x = TransControllerPose.rotation.x;
	controller_pose.rotation.y = TransControllerPose.rotation.y;
	controller_pose.rotation.z = TransControllerPose.rotation.z;
	controller_pose.rotation.w = TransControllerPose.rotation.w;
	controller_pose.timestamp = TransControllerPose.timestamp;
	controller_pose.recvTimeStampNs = nowInNs();
	controller_pose.vecAcceleration.x = TransControllerPose.vecAcceleration.x;
	controller_pose.vecAcceleration.y = TransControllerPose.vecAcceleration.y;
	controller_pose.vecAcceleration.z = TransControllerPose.vecAcceleration.z;
	controller_pose.vecAngularAcceleration.x = TransControllerPose.vecAngularAcceleration.x;
	controller_pose.vecAngularAcceleration.y = TransControllerPose.vecAngularAcceleration.y;
	controller_pose.vecAngularAcceleration.z = TransControllerPose.vecAngularAcceleration.z;
	controller_pose.vecAngularVelocity.x = TransControllerPose.vecAngularVelocity.x;
	controller_pose.vecAngularVelocity.y = TransControllerPose.vecAngularVelocity.y;
	controller_pose.vecAngularVelocity.z = TransControllerPose.vecAngularVelocity.z;
	controller_pose.vecVelocity.x = TransControllerPose.vecVelocity.x;
	controller_pose.vecVelocity.y = TransControllerPose.vecVelocity.y;
	controller_pose.vecVelocity.z = TransControllerPose.vecVelocity.z;
	if (left_or_right==0)
	{
		last_left_sdk_ts_ = controller_pose.timestamp;
	} 
	else
	{
		last_right_sdk_ts_ = controller_pose.timestamp;
	}
}
 

void TcpSensorSocket::SetHmdPose(void* data, RVR::RVRPoseHmdData& hmd_pose)
{
	hmd_pose = { 0 };
	WireLessType::TransPoseData* p_pose = (WireLessType::TransPoseData*)data;
	WireLessType::TransPoseData mTransPose = *p_pose;
	 
	mTransPose.poseRecvTime = RVR::nowInNs();
	hmd_pose.valid = true;
	hmd_pose.position.x = mTransPose.position.x;
	hmd_pose.position.y = mTransPose.position.y;
	hmd_pose.position.z = mTransPose.position.z;

	hmd_pose.rotation.x = mTransPose.rotation.x;
	hmd_pose.rotation.y = mTransPose.rotation.y;
	hmd_pose.rotation.z = mTransPose.rotation.z;
	hmd_pose.rotation.w = mTransPose.rotation.w;
	int64_t senor_index = mTransPose.poseRecvTime;
	 
	last_recv_time_ = mTransPose.poseRecvTime;
	last_hmd_sdk_ts_ = mTransPose.poseTimeStamp;

	hmd_pose.poseRecvTime = mTransPose.poseRecvTime;
	hmd_pose.poseTimeStamp = mTransPose.poseTimeStamp;
	hmd_pose.predictedTimeMs =  mTransPose.predictedTimeMs;

	hmd_pose.position.x = mTransPose.position.x;
	hmd_pose.position.y = mTransPose.position.y;
	hmd_pose.position.z = mTransPose.position.z;


	hmd_pose.linearVelocity.x = mTransPose.linearVelocity.x;
	hmd_pose.linearVelocity.y = mTransPose.linearVelocity.y;
	hmd_pose.linearVelocity.z = mTransPose.linearVelocity.z;

	hmd_pose.linearAcceleration.x = mTransPose.linearAcceleration.x;
	hmd_pose.linearAcceleration.y = mTransPose.linearAcceleration.y;
	hmd_pose.linearAcceleration.z = mTransPose.linearAcceleration.z;

	hmd_pose.angularVelocity.x = mTransPose.angularVelocity.x;
	hmd_pose.angularVelocity.y = mTransPose.angularVelocity.y;
	hmd_pose.angularVelocity.z = mTransPose.angularVelocity.z;


	hmd_pose.angularAcceleration.x = mTransPose.angularAcceleration.x;
	hmd_pose.angularAcceleration.y = mTransPose.angularAcceleration.y;
	hmd_pose.angularAcceleration.z = mTransPose.angularAcceleration.z;


}

