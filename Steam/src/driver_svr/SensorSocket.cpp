#ifndef NO_SLARDAR
#include "SlardarMoudle.h"
#endif
#include "SensorSocket.h"
#include <process.h>
#include "../RVRPlugin/RVRPluginDefinitions.h"
#include "driver_pico.h"
#include "TimeTool.h"
#include "driverlog.h"
#include "sensor_passer.h"
#include "config_reader.h"
#include <sstream>
SensorSocket* SensorSocket::gSensorSocketInstance = nullptr;
SensorSocket*SensorSocket::LeftControllerInstance = nullptr;
SensorSocket*SensorSocket::RightControllerInstance = nullptr;
SensorSocket::GC SensorSocket::SensorSocketgc;

extern ConfigReader gConfigReader;
extern PicoVRDriver g_svrDriver;
bool setip = false;
std::string gDstip="";
int g_bit_rate_update = 0;
bool g_long_sensor_ = false;
void ResetRVRControllerData(RVR::RVRControllerData& pose_data)
{
	memset(&pose_data, 0, sizeof(RVR::RVRControllerData));
	pose_data.rotation.w = 1;

};

SensorSocket::~SensorSocket()
{
	
	loop_ = false;
	closesocket(mServerSocket);
	 
}
 
unsigned int __stdcall SensorSocket::RecvThread(LPVOID lpParameter)
{
	int loop_time = 0;
	
	while (SensorSocket::GetInstance()->loop_)
	{
		bool send_encode_param = false;
		if (gConfigReader.GetLinearResolation()==1)
		{
			send_encode_param = true;
		}
		unsigned char buf[4096] = {0};
		int len= sizeof(struct sockaddr);
		int recvRet = recvfrom(SensorSocket::GetInstance()->mServerSocket, (char*)buf, 4096, 0,
			(sockaddr*)&SensorSocket::GetInstance()->m_ClientSocketaddr, &len);
		 
		if (recvRet == -1)
		{
			g_long_sensor_ = false;
			RVR::RVRPoseHmdData hmd_pose = { 0 };
			memmove(&hmd_pose, &SensorSocket::GetInstance()->hmd_pose_, sizeof(RVR::RVRPoseHmdData));
			hmd_pose.valid = FALSE;
			
			if (gConfigReader.GetSmoothControllerValue() == 1)
			{
				//SensorPasser::GetInstance()->SetHmdSensor(hmd_pose);
				//SensorSocket::GetInstance()->ResetTimeStamp();
			}
			else
			{
				if ((gConfigReader.GetRtcOrBulkMode_() == 0)&&(g_svrDriver.GetHmdActiveType_() == Hmd_Active_Type::Streaming))
				{
				//	g_svrDriver.AddHmdPose(&hmd_pose, false);
				}
			}
			
			continue;
		}
		else  //apprun:0 or1 后  hmd 一直能收到
		{
			int64_t current_time = GetTimestampInNst();
			if (current_time-SensorSocket::GetInstance(-1)->last_recv_time_>2000000000)
			{
				//SensorSocket::GetInstance()->last_time_stamp = -1;
				g_long_sensor_ = false;
				RVR::RVRPoseHmdData hmd_pose = { 0 };
				memmove(&hmd_pose, &SensorSocket::GetInstance()->hmd_pose_, sizeof(RVR::RVRPoseHmdData));
				hmd_pose.valid = FALSE;

				if (gConfigReader.GetSmoothControllerValue() == 1)
				{
					//SensorPasser::GetInstance()->SetHmdSensor(hmd_pose);
					//SensorSocket::GetInstance()->ResetTimeStamp();
				}
				else
				{
					if ((gConfigReader.GetRtcOrBulkMode_() == 0) && (g_svrDriver.GetHmdActiveType_() == Hmd_Active_Type::Streaming))
					{
					//	g_svrDriver.AddHmdPose(&hmd_pose, false);
					}
				}

			}
		}
		if (!((recvRet== NEWSENSORBUFLEN )||(recvRet==TOGETHERSENSORBUFLEN)))
		{
			std::string recv_msg;
			std::stringstream ss;
			ss << buf;
			ss >> recv_msg;
			std::string msg_head = "";
			if (recv_msg.length()>6)
			{
				msg_head = recv_msg.substr(0, 6);
			}
			if (gDstip.length() == 0)//第一次开启，通过dst sensor 或者 串流助手的通知都可以设置dst ip
			{
				gDstip = inet_ntoa(SensorSocket::GetInstance()->m_ClientSocketaddr.sin_addr);
				setip = true;
				DriverLog("set dstip  by sensor %s", gDstip.c_str());
			}

			
			if (strcmp((const char*)buf, "get_encode_param") == 0)//线性分辨率，头带端发送get_encode_param，pc端返回。
			{
			 
				WireLessType::EncodeParam  encode_param = { 0 };
				encode_param.render_width = gConfigReader.GetEveWidth();
				encode_param.render_height = gConfigReader.GetEveHeight();
				encode_param.encode_width = gConfigReader.GetEncoderWidth()/2;
				encode_param.encode_heigth = gConfigReader.GetEncoderHeight();
				encode_param.cut_x = gConfigReader.GetCutx();
				encode_param.cut_y = gConfigReader.GetCuty();
				encode_param.compress = gConfigReader.GetComPress();
				
				SensorSocket::GetInstance(-1)->SendMsg((char*)&encode_param,sizeof(encode_param), (char*)gDstip.c_str(),gConfigReader.GetPortH());
				DriverLog("encode param %d %d %d %d %d %d %d\n",
					encode_param.render_width, encode_param.render_height,
					encode_param.encode_width, encode_param.encode_heigth,
					encode_param.cut_x, encode_param.cut_y, encode_param.compress);
			}
			if (strcmp((const char*)buf,"pico_exit")==0)
			{
				//vr::VRServerDriverHost()->RequestRestart("You have requested a restart of steamvr!", NULL, NULL, NULL);
				vr::PropertyContainerHandle_t ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(g_svrDriver.GetStreamingHmdDriver()->GetVrDeviceId());
				vr::VRServerDriverHost()->VendorSpecificEvent(ulPropertyContainer, vr::VREvent_DriverRequestedQuit, {}, 0);
				
			}
		
		    if (msg_head.compare("apprun") == 0)
		    {
				std::string app_run_str= recv_msg.substr(7, recv_msg.length() - 7);
				int app_run_value = atoi(app_run_str.c_str());
				gConfigReader.SetAppRun_(app_run_value);
		    }
			if (msg_head.compare("dst_ip")==0)
			{
				DriverLog("recvbuf %s", recv_msg.c_str());
				gDstip = recv_msg.substr(7, recv_msg.length() - 7);
				setip = true;
				DriverLog("set dstip  %s  ack dst port =%u", gDstip.c_str(), 29750);
				std::string ack_msg = "dst_ip_get:";
				ack_msg = ack_msg + gDstip;
				SensorSocket::GetInstance()->SendMsg((char*)ack_msg.c_str(), ack_msg.length(),"127.0.0.1", 29750);
			}
			if (buf[0]==0x13&&buf[1] == 0xEE) // unsigned !!!!
			{
				char task_id[256] = { 0 };
				memmove(task_id, buf + 2, recvRet - 2);
				//DriverLog("get slardar task id=%s", task_id);
#ifndef NO_SLARDAR
                //update slardar task id
				SlardarMoudle::GetInstance()->SetTaskId(task_id);
				SensorSocket::GetInstance()->SendMsg((char*)buf, recvRet);
#endif
				//DriverLog("send slardar task id=%s", buf);
			}
			continue;
		}
		
		 
		if( ((buf[TOGETHERSENSORBUFLEN - 1] ==0xee)&&(buf[0] ==0x13))||
			((buf[NEWSENSORBUFLEN - 1] == 0xee) && (buf[0] == 0x13)))
		{		
			
			std::string dst_ip=inet_ntoa(SensorSocket::GetInstance()->m_ClientSocketaddr.sin_addr);
			if (dst_ip.compare(gDstip)!=0)
			{
				continue;
			}
			if (recvRet==NEWSENSORBUFLEN)
			{
				g_long_sensor_ = false;
				memset((void*)(&SensorSocket::GetInstance()->hmd_pose_), 0, sizeof(RVR::RVRPoseHmdData));
				SensorSocket::GetInstance()->SetPose((void*)(buf + 2), SensorSocket::GetInstance()->hmd_pose_);
				if (gConfigReader.GetSmoothControllerValue() == 1)//abandon
				{
					SensorPasser::GetInstance()->SetHmdSensor(SensorSocket::GetInstance()->hmd_pose_);
				}
				else
				{
					if ((gConfigReader.GetRtcOrBulkMode_() == 0) && (g_svrDriver.GetHmdActiveType_() != Hmd_Active_Type::DP))
					{
						//g_svrDriver.AddHmdPose(&SensorSocket::GetInstance()->hmd_pose_, false);
					}
				}
			}
			else
			{
				g_long_sensor_ = true;
				memset((void*)(&SensorSocket::GetInstance()->hmd_pose_), 0, sizeof(RVR::RVRPoseHmdData));
				int offset = 2;
				SensorSocket::GetInstance()->SetPose((void*)(buf + offset), SensorSocket::GetInstance()->hmd_pose_);
				offset += sizeof(WireLessType::TransPoseData);
				SensorSocket::GetInstance()->SetControllerPose((void*)(buf + offset), SensorSocket::GetInstance()->left_pose_);
				offset += sizeof(WireLessType::TransControllerData);
				SensorSocket::GetInstance()->SetControllerPose((void*)(buf + offset), SensorSocket::GetInstance()->right_pose_);

				RVR::RVRPoseHmdData hmd_pose = SensorSocket::GetInstance()->hmd_pose_;
				RVR::RVRControllerData left_pose= SensorSocket::GetInstance()->left_pose_;
				RVR::RVRControllerData right_pose= SensorSocket::GetInstance()->right_pose_;
				/*RVR_LOG_A("sensertest:%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%lld",
					hmd_pose.rotation.w, hmd_pose.rotation.x, hmd_pose.rotation.y, hmd_pose.rotation.z, hmd_pose.position.x, hmd_pose.position.y, hmd_pose.position.z,
					left_pose.rotation.w, left_pose.rotation.x, left_pose.rotation.y, left_pose.rotation.z, left_pose.position.x, left_pose.position.y, left_pose.position.z,
					right_pose.rotation.w, right_pose.rotation.x, right_pose.rotation.y, right_pose.rotation.z, right_pose.position.x, right_pose.position.y, right_pose.position.z,
					hmd_pose.poseTimeStamp);*/

			/*	if ((gConfigReader.GetRtcOrBulkMode_() == 0) && (g_svrDriver.GetHmdActiveType_() != Hmd_Active_Type::DP))
				{
					if (gConfigReader.GetSmoothControllerValue() == 2)
					{
						g_svrDriver.sensor_queue_.SetPose(SensorSocket::GetInstance()->hmd_pose_, SensorSocket::GetInstance()->left_pose_, SensorSocket::GetInstance()->right_pose_);
					}
					else if (gConfigReader.GetSmoothControllerValue() == 0)
					{
						g_svrDriver.AddHmdPose(&SensorSocket::GetInstance()->hmd_pose_, false);
						g_svrDriver.AddControllerPose((uint32_t)ControllerIndex::kLeftController, &SensorSocket::GetInstance()->left_pose_);
						g_svrDriver.AddControllerPose((uint32_t)ControllerIndex::kRightController, &SensorSocket::GetInstance()->right_pose_);
					}
					else if (gConfigReader.GetSmoothControllerValue() == 3 || gConfigReader.GetSmoothControllerValue() == 5)
					{
						if (g_svrDriver.GetStreamingHmdDriver()->GetIsAdded_()==false)
						{
							g_svrDriver.AddHmdPose(&SensorSocket::GetInstance()->hmd_pose_, false);
							g_svrDriver.AddControllerPose((uint32_t)ControllerIndex::kLeftController, &SensorSocket::GetInstance()->left_pose_);
							g_svrDriver.AddControllerPose((uint32_t)ControllerIndex::kRightController, &SensorSocket::GetInstance()->right_pose_);
						}
						SensorPasser::GetInstance()->SetHmdSensor(SensorSocket::GetInstance()->hmd_pose_);
						SensorPasser::GetInstance()->SetControllerSensor(SensorSocket::GetInstance()->left_pose_,ControllerIndex::kLeftController);
						SensorPasser::GetInstance()->SetControllerSensor(SensorSocket::GetInstance()->right_pose_, ControllerIndex::kRightController);
						SensorPasser::GetInstance()->SetNewSensorNotifyEvent();
					}
					else if (gConfigReader.GetSmoothControllerValue() == 6)
					{
						if (g_svrDriver.GetStreamingHmdDriver()->GetIsAdded_() == false)
						{
							g_svrDriver.AddHmdPose(&SensorSocket::GetInstance()->hmd_pose_, false);
							g_svrDriver.AddControllerPose((uint32_t)ControllerIndex::kLeftController, &SensorSocket::GetInstance()->left_pose_);
							g_svrDriver.AddControllerPose((uint32_t)ControllerIndex::kRightController, &SensorSocket::GetInstance()->right_pose_);
						}
						SensorPasser::GetInstance()->SetAllSensor(SensorSocket::GetInstance()->hmd_pose_, SensorSocket::GetInstance()->left_pose_, SensorSocket::GetInstance()->right_pose_);
						 
						 
					}
					else if (gConfigReader.GetSmoothControllerValue() == 4)
					{
						if (g_svrDriver.GetStreamingHmdDriver()->GetIsAdded_() == false)
						{
							g_svrDriver.AddHmdPose(&SensorSocket::GetInstance()->hmd_pose_, false);
							g_svrDriver.AddControllerPose((uint32_t)ControllerIndex::kLeftController, &SensorSocket::GetInstance()->left_pose_);
							g_svrDriver.AddControllerPose((uint32_t)ControllerIndex::kRightController, &SensorSocket::GetInstance()->right_pose_);
						}
						g_svrDriver.sensor_queue_.SetPose(SensorSocket::GetInstance()->hmd_pose_, SensorSocket::GetInstance()->left_pose_, SensorSocket::GetInstance()->right_pose_);
					}

				}*/
			}
			
			
			
		}
		else
		{
			DriverLog("socket error\n");;
		}
	}
	return 0;
}

void SensorSocket::SetControllerPose(void* data, RVR::RVRControllerData& controller_pose)
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
void SensorSocket::GetPose(WireLessType::TransPoseData &pose)
{ 
	pose = mTransPose; 
}

void SensorSocket::SetPose(void * data, RVR::RVRPoseHmdData& hmd_pose)
{
	hmd_pose = { 0 };
	WireLessType::TransPoseData *p_pose = (WireLessType::TransPoseData*)data;
	mTransPose = *p_pose;
	//if (last_time_stamp < mTransPose.poseTimeStamp)//????poseRecvTime
	//{
	//	((SensorSocket*)SensorSocket::GetInstance())->last_time_stamp = mTransPose.poseTimeStamp;
	//}
	//else
	//{
	//	((SensorSocket*)SensorSocket::GetInstance())->last_time_stamp = mTransPose.poseTimeStamp;
	// 
	//	return;
	//}

 //
	if (base_sdk_time_ < 0)
	{
		base_sdk_time_ = mTransPose.poseTimeStamp ;
	}

	hmd_pose.valid = true;
	hmd_pose.position.x = mTransPose.position.x;
	hmd_pose.position.y = mTransPose.position.y;
	hmd_pose.position.z = mTransPose.position.z;

	hmd_pose.rotation.x = mTransPose.rotation.x;
	hmd_pose.rotation.y = mTransPose.rotation.y;
	hmd_pose.rotation.z = mTransPose.rotation.z;
	hmd_pose.rotation.w = mTransPose.rotation.w;
	int64_t senor_index = mTransPose.poseRecvTime;
    SensorSocket::GetInstance()->mTransPose.poseRecvTime = GetTimestampInNst();
	int64_t recv_space = SensorSocket::GetInstance()->mTransPose.poseRecvTime - last_recv_time_;
	int64_t sdk_spcce = SensorSocket::GetInstance()->mTransPose.poseTimeStamp - last_sdk_time_;
	if (hmd_pose.poseRecvTime!=0)
	{
		if (((recv_space - sdk_spcce) > 15000000)||((senor_index-last_sensor_index_)>3))
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
	last_recv_time_ = SensorSocket::GetInstance()->mTransPose.poseRecvTime;
	last_sdk_time_ = SensorSocket::GetInstance()->mTransPose.poseTimeStamp;
	last_sensor_index_= senor_index;
	hmd_pose.poseRecvTime = SensorSocket::GetInstance()->mTransPose.poseRecvTime;
	hmd_pose.poseTimeStamp = SensorSocket::GetInstance()->mTransPose.poseTimeStamp;
	hmd_pose.predictedTimeMs = SensorSocket::GetInstance()->mTransPose.predictedTimeMs;
	
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


SensorSocket * SensorSocket::GetInstance(int index)
{

	if (index < 0)
	{
		if (gSensorSocketInstance == NULL)
		{
			gSensorSocketInstance = new SensorSocket();
			gSensorSocketInstance->mIndex = -1;
		}
		return gSensorSocketInstance;
	}
	if (index == 0)
	{
		if (LeftControllerInstance == NULL)
		{
			LeftControllerInstance = new ControllerSensorSocket();
			LeftControllerInstance->mIndex = 0;

		}
		return LeftControllerInstance;
	}
	if (index == 1)
	{
		if (RightControllerInstance == NULL)
		{
			RightControllerInstance = new ControllerSensorSocket();
			RightControllerInstance->mIndex = 1;
		}
		return RightControllerInstance;
	}

}

int SensorSocket::InitSocket(char * ip, u_short port )
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

	//int value =1024*1024*16;
	//setsockopt(mServerSocket, SOL_SOCKET, SO_RCVBUF, (char*)&value, sizeof(value));
	struct sockaddr_in Local_Addr;
	Local_Addr.sin_family = AF_INET;
	Local_Addr.sin_port = htons(port);
	Local_Addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(mServerSocket, (struct sockaddr *)&Local_Addr, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
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
	loop_ = true;
	if (mIndex==-1)
	{
		 
		HANDLE ret = (HANDLE)_beginthreadex(NULL, 0, &RecvThread, this, 0, NULL);
		BOOL set_ret = SetThreadPriority(ret, THREAD_PRIORITY_TIME_CRITICAL);
		
	} 
	else
	{
		//HANDLE ret = (HANDLE)_beginthreadex(NULL, 0, &((ControllerSensorSocket*)SensorSocket::GetInstance(mIndex))->RecvThread, this, 0, NULL);
		//BOOL set_ret = SetThreadPriority(ret, THREAD_PRIORITY_TIME_CRITICAL);
	}
	
	return 1;

}

int SensorSocket::SendMsg(char * buf, int len)
{
	m_ClientSocketaddr.sin_family = AF_INET;
	m_ClientSocketaddr.sin_port = htons(29722);
	m_ClientSocketaddr.sin_addr.S_un.S_addr = inet_addr(gDstip.c_str());
	int ret=sendto(mServerSocket, buf, len, 0, (sockaddr*)&m_ClientSocketaddr, sizeof(m_ClientSocketaddr));
	return ret;
}
int SensorSocket::SendMsg(char* buf, int len, char* ip, u_short port)
{
	sockaddr_in dst_addr;
	dst_addr.sin_port = htons(port);
	dst_addr.sin_family = AF_INET;
	dst_addr.sin_addr.s_addr = inet_addr(ip);
	int ret=sendto(mhelpSocket, buf, len, 0, (sockaddr*)&dst_addr, sizeof(dst_addr));
	if (ret<=0)
	{
		DriverLog("SendMsg to 127.0.0.1 error");
	}
	return 0;
}
int SensorSocket::ShutDown() 
{
	loop_ = false;
	closesocket(mServerSocket);
	closesocket(mhelpSocket);
	return 1;
}
ControllerSensorSocket::~ControllerSensorSocket()
{
	loop_ = false;
	closesocket(mServerSocket);
}

unsigned int __stdcall ControllerSensorSocket::RecvThread(LPVOID lpParameter)
{
	 
	SensorSocket *pobj = (SensorSocket*)lpParameter;
	
	unsigned char buf[CONTROLLERSENSORBUFLEN] = {0};
	int offset = 0;
	 
	while (SensorSocket::GetInstance(pobj->mIndex)->loop_)
	{
		 
		int len = sizeof(struct sockaddr);
		int recvRet = recvfrom(SensorSocket::GetInstance(pobj->mIndex)->mServerSocket, (char*)(buf + offset), CONTROLLERSENSORBUFLEN - offset, 0,
			(sockaddr*)&SensorSocket::GetInstance(pobj->mIndex)->m_ClientSocketaddr, &len);

		if (recvRet == -1) {	
			if (g_long_sensor_)
			{
				continue;
			}
			SensorSocket::GetInstance(pobj->mIndex)->ResetTimeStamp();
			RVR::RVRControllerData controller_pose = {0};
			//memmove(&controller_pose, &((ControllerSensorSocket*)SensorSocket::GetInstance(pobj->mIndex))->controller_pose_, sizeof(RVR::RVRControllerData));
			controller_pose.rotation.w = 1;
			controller_pose.connectionState=RVR::RVRControllerConnectionState::kDisconnected;
			if (gConfigReader.GetSmoothControllerValue() == 1)
			{
				SensorPasser::GetInstance()->SetControllerSensor(controller_pose, ControllerIndex(pobj->mIndex));
			} 
			else
			{

				if ((gConfigReader.GetRtcOrBulkMode_() == 0) && (g_svrDriver.GetHmdActiveType_() == Hmd_Active_Type::Streaming))
				{
					g_svrDriver.AddControllerPose(pobj->mIndex, &controller_pose);
				}
				
			}
			
			continue;
		}
		if (recvRet != CONTROLLERSENSORBUFLEN  )
		{
			continue;
		}
		if (buf[0] == 0x13 && buf[CONTROLLERSENSORBUFLEN - 1] == 0xEE)
		{
			std::string dst_ip = inet_ntoa(SensorSocket::GetInstance(pobj->mIndex)->m_ClientSocketaddr.sin_addr);
			if (dst_ip.compare(gDstip) != 0)
			{
				continue;
			}
			memset(&((ControllerSensorSocket*)SensorSocket::GetInstance(pobj->mIndex))->controller_pose_, 0, sizeof(RVR::RVRControllerData));
			((ControllerSensorSocket*)SensorSocket::GetInstance(pobj->mIndex))->SetPose((void*)(buf + 2), ((ControllerSensorSocket*)SensorSocket::GetInstance(pobj->mIndex))->controller_pose_);
			if (gConfigReader.GetSmoothControllerValue()==1)
			{
				SensorPasser::GetInstance()->SetControllerSensor(((ControllerSensorSocket*)SensorSocket::GetInstance(pobj->mIndex))->controller_pose_, ControllerIndex(pobj->mIndex));
			} 
			else
			{
				if ((gConfigReader.GetRtcOrBulkMode_() == 0) && (g_svrDriver.GetHmdActiveType_() != Hmd_Active_Type::DP))
				{
					g_svrDriver.AddControllerPose(pobj->mIndex, &((ControllerSensorSocket*)SensorSocket::GetInstance(pobj->mIndex))->controller_pose_);
				}
			}
		

			
		}
		else
		{
			DriverLog("socket error\n");
		}
	}
	return 0;
	
}

void ControllerSensorSocket::GetPose(WireLessType::TransControllerData & pose)
{
 
	pose = mTransControllerPose;
	 
}

void ControllerSensorSocket::SetPose(void* data, RVR::RVRControllerData &controller_sensor)
{
	WireLessType::TransControllerData* p_pose = (WireLessType::TransControllerData*)data;

	mTransControllerPose = *p_pose;
	 
	
	if (base_sdk_time_ < 0)
	{
		base_sdk_time_ = mTransControllerPose.timestamp;
	}

	controller_sensor = {0};

	memmove(&controller_sensor.analog1D, &mTransControllerPose.analog1D, sizeof(float) * 8);
	memmove(&controller_sensor.analog2D, &mTransControllerPose.analog2D, sizeof(RVRVector2) * 4);
	controller_sensor.buttonState = mTransControllerPose.buttonState;
	controller_sensor.connectionState = (RVR::RVRControllerConnectionState)mTransControllerPose.connectionState;
	controller_sensor.isTouching = mTransControllerPose.isTouching;
	controller_sensor.position.x = mTransControllerPose.position.x;
	controller_sensor.position.y = mTransControllerPose.position.y;
	controller_sensor.position.z = mTransControllerPose.position.z;
	controller_sensor.rotation.x = mTransControllerPose.rotation.x;
	controller_sensor.rotation.y = mTransControllerPose.rotation.y;
	controller_sensor.rotation.z = mTransControllerPose.rotation.z;
	controller_sensor.rotation.w = mTransControllerPose.rotation.w;
	controller_sensor.timestamp = mTransControllerPose.timestamp;
	controller_sensor.recvTimeStampNs = nowInNs();
	controller_sensor.vecAcceleration.x = mTransControllerPose.vecAcceleration.x;
	controller_sensor.vecAcceleration.y = mTransControllerPose.vecAcceleration.y;
	controller_sensor.vecAcceleration.z = mTransControllerPose.vecAcceleration.z;
	controller_sensor.vecAngularAcceleration.x = mTransControllerPose.vecAngularAcceleration.x;
	controller_sensor.vecAngularAcceleration.y = mTransControllerPose.vecAngularAcceleration.y;
	controller_sensor.vecAngularAcceleration.z = mTransControllerPose.vecAngularAcceleration.z;
	controller_sensor.vecAngularVelocity.x = mTransControllerPose.vecAngularVelocity.x;
	controller_sensor.vecAngularVelocity.y = mTransControllerPose.vecAngularVelocity.y;
	controller_sensor.vecAngularVelocity.z = mTransControllerPose.vecAngularVelocity.z;
	controller_sensor.vecVelocity.x = mTransControllerPose.vecVelocity.x;
	controller_sensor.vecVelocity.y = mTransControllerPose.vecVelocity.y;
	controller_sensor.vecVelocity.z = mTransControllerPose.vecVelocity.z;
	

	/*std::string msg = "mindex=";
	msg = msg + std::to_string(mIndex);
	msg = msg + ",connectionState" + std::to_string((int)tmplog.connectionState);
	msg = msg + ",position:" + std::to_string(tmplog.position.x)+","
		+ std::to_string(tmplog.position.y) + "," + std::to_string(tmplog.position.z);
	msg = msg + ",rotation:" + std::to_string(tmplog.rotation.x) + "," +
		std::to_string(tmplog.rotation.y) + "," + std::to_string(tmplog.rotation.z) +
		","+std::to_string(tmplog.rotation.w);
	msg = msg + ",vecVelocity" + std::to_string(tmplog.vecVelocity.x)+","+
		std::to_string(tmplog.vecVelocity.y)+","+ std::to_string(tmplog.vecVelocity.z);

	msg = msg + ",vecAngularVelocity" + std::to_string(tmplog.vecAngularVelocity.x) + "," +
		std::to_string(tmplog.vecAngularVelocity.y) + "," + std::to_string(tmplog.vecAngularVelocity.z);
	msg = msg + ",vecAcceleration" + std::to_string(tmplog.vecAcceleration.x) + "," +
		std::to_string(tmplog.vecAcceleration.y) + "," + std::to_string(tmplog.vecAcceleration.z);
	msg = msg + "\n";
	DriverLog(msg.c_str());*/
}
