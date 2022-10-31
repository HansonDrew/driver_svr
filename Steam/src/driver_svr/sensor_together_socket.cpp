#ifndef NO_SLARDAR
#include "SlardarMoudle.h"
#endif
#include "sensor_together_socket.h"
#include <process.h>
#include "../RVRPlugin/RVRPluginDefinitions.h"
#include "driver_pico.h"
#include "TimeTool.h"
#include "driverlog.h"
#include "sensor_passer.h"
#include "config_reader.h"
#include <sstream>

sensor_together_socket* sensor_together_socket::g_sensor_together_socket_ = nullptr; 
sensor_together_socket::GC sensor_together_socket::SensorSocketgc;

extern ConfigReader gConfigReader;
extern PicoVRDriver g_svrDriver;
extern bool setip  ;
extern std::string gDstip  ;
extern int g_bit_rate_update  ;

void sensor_together_socket::ResetRVRControllerData(RVR::RVRControllerData& pose_data)
{
	memset(&pose_data, 0, sizeof(RVR::RVRControllerData));
	pose_data.rotation.w = 1;

};

sensor_together_socket::~sensor_together_socket()
{

	RecvLoop = false;
	closesocket(mServerSocket);

}

unsigned int __stdcall sensor_together_socket::RecvThread(LPVOID lpParameter)
{
	int loop_time = 0;

	while (sensor_together_socket::GetInstance()->RecvLoop)
	{
		bool send_encode_param = false;
		if (gConfigReader.GetLinearResolation() == 1)
		{
			send_encode_param = true;
		}
		char buf[4096] = { 0 };
		int len = sizeof(struct sockaddr);
		int recvRet = recvfrom(sensor_together_socket::GetInstance()->mServerSocket, buf, 4096, 0,
			(sockaddr*)&sensor_together_socket::GetInstance()->m_ClientSocketaddr, &len);

		if (recvRet == -1)
		{
			RVR::RVRPoseHmdData hmd_pose = { 0 };
			memmove(&hmd_pose, &sensor_together_socket::GetInstance()->hmd_pose_, sizeof(RVR::RVRPoseHmdData));
			hmd_pose.valid = FALSE;
			
			if ((gConfigReader.GetRtcOrBulkMode_() == 0) && (g_svrDriver.GetHmdActiveType_() == Hmd_Active_Type::Streaming))
			{
				RVR::RVRControllerData controller_pose = { 0 };
				controller_pose.rotation.w = 1;
				controller_pose.connectionState = RVR::RVRControllerConnectionState::kDisconnected;
				g_svrDriver.AddHmdPose(&hmd_pose, false);
				g_svrDriver.AddControllerPose((uint32_t)ControllerIndex::kLeftController, &controller_pose);
				g_svrDriver.AddControllerPose((uint32_t)ControllerIndex::kRightController, &controller_pose);
			}
			
			continue;
		}
		if (!((recvRet ==  TOGETHERSENSORBUFLEN)))
		{
			std::string recv_msg;
			std::stringstream ss;
			ss << buf;
			ss >> recv_msg;
			std::string msg_head = "";
			if (recv_msg.length() > 6)
			{
				msg_head = recv_msg.substr(0, 6);
			}

			if (strcmp((const char*)buf, "get_encode_param") == 0)//线性分辨率，头带端发送get_encode_param，pc端返回。
			{
				send_encode_param = false;
			}
			if (send_encode_param)
			{
				WireLessType::EncodeParam  encode_param = { 0 };
				encode_param.render_width = gConfigReader.GetEveWidth();
				encode_param.render_height = gConfigReader.GetEveHeight();
				encode_param.encode_width = gConfigReader.GetEncoderWidth();
				encode_param.encode_heigth = gConfigReader.GetEncoderHeight();
				encode_param.cut_x = gConfigReader.GetCutx();
				encode_param.cut_y = gConfigReader.GetCuty();
				encode_param.compress = gConfigReader.GetComPress();
				sensor_together_socket::GetInstance()->SendMsg((char*)&encode_param, sizeof(encode_param), (char*)gDstip.c_str(), gConfigReader.GetPortH());
			}
			if (strcmp((const char*)buf, "pico_exit") == 0)
			{
				vr::VRServerDriverHost()->RequestRestart("You have requested a restart of steamvr!", NULL, NULL, NULL);
			}

			if (msg_head.compare("dst_ip") == 0)
			{
				DriverLog("recvbuf %s", recv_msg.c_str());
				gDstip = recv_msg.substr(7, recv_msg.length() - 7);
				setip = true;
				DriverLog("set dstip  %s  ack dst port =%u", gDstip.c_str(), 29750);
				std::string ack_msg = "dst_ip_get:";
				ack_msg = ack_msg + gDstip;
				sensor_together_socket::GetInstance()->SendMsg((char*)ack_msg.c_str(), ack_msg.length(), "127.0.0.1", 29750);
			}
			
			if ((unsigned char)buf[0] == 0x13 && (unsigned char)buf[1] == 0xEE) // unsigned !!!!
			{

				
				char task_id[256] = { 0 };
				memmove(task_id, buf + 2, recvRet - 2);
				DriverLog("get slardar task id=%s", task_id);
#ifndef NO_SLARDAR
				//update slardar task id
				SlardarMoudle::GetInstance()->SetTaskId(task_id);
				sensor_together_socket::GetInstance()->SendMsg((char*)buf, recvRet);
#endif
				DriverLog("send slardar task id=%s", buf);
			}
			continue;
		}
		if (gDstip.length() == 0)//第一次开启，通过dst sensor 或者 串流助手的通知都可以设置dst ip
		{
			gDstip = inet_ntoa(sensor_together_socket::GetInstance()->m_ClientSocketaddr.sin_addr);
			setip = true;
			DriverLog("set dstip  by sensor %s", gDstip.c_str());
		}

		if (((unsigned char)buf[TOGETHERSENSORBUFLEN - 1] == 0xee) && ((unsigned char)buf[0] == 0x13))
		{
			if (recvRet == TOGETHERSENSORBUFLEN)
			{
				buf[TOGETHERSENSORBUFLEN - 1] = 0;
			}
			std::string dst_ip = inet_ntoa(sensor_together_socket::GetInstance()->m_ClientSocketaddr.sin_addr);
			if (dst_ip.compare(gDstip) != 0)
			{
				continue;
			}
			memset((void*)(&sensor_together_socket::GetInstance()->hmd_pose_), 0, sizeof(RVR::RVRPoseHmdData));
			int offset = 2;
			sensor_together_socket::GetInstance()->SetHmdPose((void*)(buf + offset), sensor_together_socket::GetInstance()->hmd_pose_);
			offset += sizeof(WireLessType::TransPoseData);
			sensor_together_socket::GetInstance()->SetControllerPose((void*)(buf + offset), sensor_together_socket::GetInstance()->left_pose_);
			offset += sizeof(WireLessType::TransControllerData);
			sensor_together_socket::GetInstance()->SetControllerPose((void*)(buf + offset), sensor_together_socket::GetInstance()->right_pose_);
			if ((gConfigReader.GetRtcOrBulkMode_() == 0) && (g_svrDriver.GetHmdActiveType_() != Hmd_Active_Type::DP))
			{
				if (gConfigReader.GetSmoothControllerValue()==2)
				{
					g_svrDriver.sensor_queue_.SetPose(sensor_together_socket::GetInstance()->hmd_pose_, sensor_together_socket::GetInstance()->left_pose_, sensor_together_socket::GetInstance()->right_pose_);
				} 
				else
				{
					g_svrDriver.AddHmdPose(&sensor_together_socket::GetInstance()->hmd_pose_, false);
					g_svrDriver.AddControllerPose((uint32_t)ControllerIndex::kLeftController, &sensor_together_socket::GetInstance()->left_pose_);
					g_svrDriver.AddControllerPose((uint32_t)ControllerIndex::kRightController, &sensor_together_socket::GetInstance()->right_pose_);
				}
				
			}
					
		}
		else
		{
			DriverLog("socket error\n");;
		}
	}
	return 0;
}


void sensor_together_socket::SetHmdPose(void* data, RVR::RVRPoseHmdData& hmd_pose, bool realy_update)
{
	hmd_pose = { 0 };
	WireLessType::TransPoseData* p_pose = (WireLessType::TransPoseData*)data;
	WireLessType::TransPoseData trans_pose;
	trans_pose = *p_pose;
	 
	if (base_sdk_time_ < 0)
	{
		base_sdk_time_ = trans_pose.pose_timestamp_ns;
	}

	hmd_pose.valid = true;
	hmd_pose.position.x = trans_pose.position.x;
	hmd_pose.position.y = trans_pose.position.y;
	hmd_pose.position.z = trans_pose.position.z;

	hmd_pose.rotation.x = trans_pose.rotation.x;
	hmd_pose.rotation.y = trans_pose.rotation.y;
	hmd_pose.rotation.z = trans_pose.rotation.z;
	hmd_pose.rotation.w = trans_pose.rotation.w;
	int64_t senor_index = trans_pose.pose_index;
	hmd_pose.poseRecvTime= GetTimestampInNst();
	int64_t recv_space = hmd_pose.poseRecvTime - last_recv_time_;
	int64_t sdk_spcce = trans_pose.pose_timestamp_ns - last_sdk_time_;
	
	if (recv_space>6000000)
	{
		DriverLog("hmd recvspace %lld  sdkspace=%lld",recv_space, sdk_spcce);
	}

	if (senor_index<last_sensor_index_)
	{
		DriverLog("hmd recvspace %lld  sdkspace=%lld index=%d lastinedx=%d", recv_space, sdk_spcce,senor_index,last_sensor_index_);
	}
	if (hmd_pose.poseRecvTime != 0)
	{
		if (((recv_space - sdk_spcce) > 15000000) || ((senor_index - last_sensor_index_) > 3))
		{
			g_bit_rate_update = -1;//用 sensor 间隔来判断是否下降码率，如果sensor间隔大，能推测网络不正常
			/*RVR_LOG_A("hmd recvthread space %lld  index sub= %lld ,index=%lld",
				recv_space, senor_index - last_sensor_index_,
				sdk_spcce);
			DriverLog("hmd recvthread space %lld  index sub= %lld ,index=%lld",
				recv_space, senor_index - last_sensor_index_,
				sdk_spcce);*/
		}
	}
	last_recv_time_ = hmd_pose.poseRecvTime;
	last_sdk_time_ = trans_pose.pose_timestamp_ns;
	last_sensor_index_ = senor_index;
	
	hmd_pose.pose_timestamp_ns = trans_pose.pose_timestamp_ns;
	hmd_pose.predictedTimeMs = trans_pose.predictedTimeMs;

	hmd_pose.position.x = trans_pose.position.x;
	hmd_pose.position.y = trans_pose.position.y;
	hmd_pose.position.z = trans_pose.position.z;


	hmd_pose.linearVelocity.x = trans_pose.linearVelocity.x;
	hmd_pose.linearVelocity.y = trans_pose.linearVelocity.y;
	hmd_pose.linearVelocity.z = trans_pose.linearVelocity.z;

	hmd_pose.linearAcceleration.x = trans_pose.linearAcceleration.x;
	hmd_pose.linearAcceleration.y = trans_pose.linearAcceleration.y;
	hmd_pose.linearAcceleration.z = trans_pose.linearAcceleration.z;

	hmd_pose.angularVelocity.x = trans_pose.angularVelocity.x;
	hmd_pose.angularVelocity.y = trans_pose.angularVelocity.y;
	hmd_pose.angularVelocity.z = trans_pose.angularVelocity.z;


	hmd_pose.angularAcceleration.x = trans_pose.angularAcceleration.x;
	hmd_pose.angularAcceleration.y = trans_pose.angularAcceleration.y;
	hmd_pose.angularAcceleration.z = trans_pose.angularAcceleration.z;


}

void sensor_together_socket::SetControllerPose(void* data, RVR::RVRControllerData& controller_pose) 
{
	WireLessType::TransControllerData* p_pose = (WireLessType::TransControllerData*)data;

	WireLessType::TransControllerData TransControllerPose = *p_pose;


	if (base_sdk_time_ < 0)
	{
		base_sdk_time_ = TransControllerPose.timestamp;
	}

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
}

sensor_together_socket* sensor_together_socket::GetInstance( )
{

	if (g_sensor_together_socket_ == NULL)
	{
		g_sensor_together_socket_ = new sensor_together_socket();
		g_sensor_together_socket_->mIndex = -1;
	}
	return g_sensor_together_socket_;
	
}

int sensor_together_socket::InitSocket(char* ip, u_short port)
{

	mServerSocket = socket(AF_INET, SOCK_DGRAM, 0);
	std::string msg = "socketport:";
	msg = msg + std::to_string(port);
	DriverLog(msg.c_str());
	if (mServerSocket == INVALID_SOCKET)
	{
		DriverLog("socket init error");
		return -1;
	}

	char	chOptVal = 1;
	if (setsockopt(mServerSocket, SOL_SOCKET, SO_REUSEADDR, &chOptVal, sizeof(char)) == SOCKET_ERROR)
	{
		DriverLog("端口复用设置失败");
	}
	int time_over = 2000;
	if (setsockopt(mServerSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&time_over, sizeof(int)) == SOCKET_ERROR)
	{
		DriverLog("端口超时设置失败");
	}

	char value = 1024 * 64;
	setsockopt(mServerSocket, SOL_SOCKET, SO_RCVBUF, (char*)&value, sizeof(value));
	struct sockaddr_in Local_Addr;
	Local_Addr.sin_family = AF_INET;
	Local_Addr.sin_port = htons(port);
	Local_Addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(mServerSocket, (struct sockaddr*)&Local_Addr, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		DriverLog("绑定失败");
	}

	//////////////////////////////////////////////////////////////////////////

	mhelpSocket = socket(AF_INET, SOCK_DGRAM, 0);

	DriverLog(msg.c_str());
	if (mhelpSocket == INVALID_SOCKET)
	{
		DriverLog("mhelpSocket init error");
		return -1;
	}

	//char value =1024;
	//setsockopt(mServerSocket, SOL_SOCKET, SO_RCVBUF, (char*)&value, sizeof(value));

	/////
	/*m_ClientSocketaddr.sin_family = AF_INET;
	m_ClientSocketaddr.sin_port = htons(port);
	m_ClientSocketaddr.sin_addr.S_un.S_addr = inet_addr((const char*)ip);*/
	RecvLoop = true;
	HANDLE ret = (HANDLE)_beginthreadex(NULL, 0, &RecvThread, this, 0, NULL);

	return 1;
}

int sensor_together_socket::SendMsg(char* buf, int len)
{
	m_ClientSocketaddr.sin_port = htons(29722);
	int ret = sendto(mServerSocket, buf, len, 0, (sockaddr*)&m_ClientSocketaddr, sizeof(m_ClientSocketaddr));
	return ret;
}
int sensor_together_socket::SendMsg(char* buf, int len, char* ip, u_short port)
{
	sockaddr_in dst_addr;
	dst_addr.sin_port = htons(port);
	dst_addr.sin_family = AF_INET;
	dst_addr.sin_addr.s_addr = inet_addr(ip);
	int ret = sendto(mhelpSocket, buf, len, 0, (sockaddr*)&dst_addr, sizeof(dst_addr));
	if (ret <= 0)
	{
		DriverLog("SendMsg to 127.0.0.1 error");
	}
	return 0;
}
