#include "hid_module.h"

#include "chaperone_data.h"
#include <windows.h> 
#include "driverlog.h"
#include "TimeTool.h"
#include "driver_pico.h"
#include "driver_define.h" 
 
extern PicoVRDriver g_svrDriver;
extern bool gLog;
HidModule* HidModule::HidModuleInstance;
HidModule::GC HidModule::HidModuleGc;
HidModule::HidModule()
{
}

HidModule::~HidModule()
{
	
}
HidModule* HidModule::GetInstance() 
{
	if (HidModuleInstance==NULL)
	{
		HidModuleInstance = new HidModule();
		HidModuleInstance->SetLoop_(true);
	}
	return HidModuleInstance;
}
void HidModule::CleanUp() 
{
	PostThreadMessage(chaper_one_threadid_, OUTTHREADMSG, NULL, NULL);
	SetLoop_(false);
	WaitForSingleObject(read_thread_event_, INFINITE);
	WaitForSingleObject(check_thread_event_, INFINITE);
}
int HidModule::InitHidDevice()
{
	InitializeCriticalSection(&m_mutex);
	int ret = hid_init();
	return ret;
}
int HidModule::WriteData(unsigned char* buf, size_t size)
{
	unsigned char send_buf[64] = { 0 };
	send_buf[0] = 0xAA;
	memmove(send_buf + 1, buf, size);
	int retval;
	EnterCriticalSection(&m_mutex);

	retval = hid_write(m_Hidhandle, send_buf, sizeof(send_buf));
	LeaveCriticalSection(&m_mutex);
	return retval;

}
int HidModule::OpenHidDevice(const unsigned short vid, const unsigned short pid, const wchar_t* serial_number_)
{
	EnterCriticalSection(&m_mutex);
	m_Hidhandle = hid_open(vid, pid, serial_number_);
	LeaveCriticalSection(&m_mutex);
	if (NULL != m_Hidhandle)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int HidModule::ReadData(unsigned char* buf, size_t size)
{
	int ret = -1;

	EnterCriticalSection(&m_mutex);
	if (!m_Hidhandle)
	{
		LeaveCriticalSection(&m_mutex);
		return -1;
	}
	ret = hid_read_timeout(m_Hidhandle, buf, size,500);

	LeaveCriticalSection(&m_mutex);


	return ret;
}
int HidModule::OperateHidDate(HidType::HidCommon hidcommon)
{
	
	if ((HidType::MessageType)hidcommon.devState.type == HidType::MessageType::kHmdData)
	{
		last_hmd_recv_timestamp = RVR::nowInNs();
		OperateHmdData(hidcommon);
	}
	else if ((HidType::MessageType)hidcommon.devState.type == HidType::MessageType::kLeftControllerData||
		(HidType::MessageType)hidcommon.devState.type == HidType::MessageType::kRightControllerData)
	{
		OperateControllerData(hidcommon);
	}
	else if ((HidType::MessageType)hidcommon.devState.type == HidType::MessageType::kDistortionOffset)
	{
		OperateDistortionOffser(hidcommon);
	}
	else if (((HidType::MessageType)hidcommon.devState.type == HidType::MessageType::kChaperOne)&& (ChaperOneData::GetInstance()->GetAllDate() == false))//获取安全区数据
	{
		DriverLog("get chaperone data");
		ChaperOneData::GetInstance()->OperateData((unsigned char*)(&hidcommon), 63);
		if (ChaperOneData::GetInstance()->CheckGetAllData())
		{
			/*RVR::RVRPoseHmdData hmd_pose = { 0 };
			hmd_pose.valid = true;
			hmd_pose.rotation.w = 1;
			g_svrDriver.AddHmdPose(&hmd_pose,true);*/
			PostThreadMessage(HidModule::GetInstance()->GetChaperOneSetThreadId_(),ChaperOneMsg,NULL,NULL);
			ChaperOneData::GetInstance()->RequestChaperOneData(HidType::ChaperOneRequestType::kGetEnd);
			DriverLog("get chaperone data end request sensor");
		}
	}
	return 1;
}

unsigned int __stdcall HidModule::ChaperOneSetThread(LPVOID lpParameter)
{
	MSG msg;
	 
	PeekMessage(&msg, NULL, ChaperOneMsg, OUTTHREADMSG, PM_NOREMOVE);
	while (HidModule::GetInstance()->GetLoop_())
	{
		if (GetMessage(&msg, nullptr, ChaperOneMsg, OUTTHREADMSG)) //get msg from message queue
		{
			switch (msg.message)
			{
			case ChaperOneMsg:
			{
				while (g_svrDriver.GetDpHmdDriver()->GetIsActived_() != true)
				{
					Sleep(1);
				}
				Sleep(100);
				ChaperOneData::GetInstance()->SetChaperOne();
				break;
				
			}
			case OUTTHREADMSG: 
			{
				return 1;
			}
			}
		}
	}
	return 1;
}

void HidModule::SetCallbackFunction(std::function<void()> exec)
{
	this->m_CallBackExec = exec;
}

int HidModule::CloseHidDevice()
{
	DeleteCriticalSection(&m_mutex);
	if (m_Hidhandle)
	{
		hid_close(m_Hidhandle);
	}
	try
	{
		hid_exit();
	}
	catch (...)
	{

	}
	
	
	return 0;
}
void HidModule::StartUp() 
{
	read_thread_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	check_thread_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	SetLoop_(true);
	
	_beginthreadex(NULL, 0, &ReadHmiDataThread, this, 0, 0);
	_beginthreadex(NULL, 0, &CheckHmiDataThread, this, 0, 0);
	_beginthreadex(NULL, 0, &ChaperOneSetThread,this, 0, &chaper_one_threadid_);
}

unsigned int __stdcall HidModule::CheckHmiDataThread(LPVOID lpParameter)
{
	int64_t hid_timestamp_log = -1;
	while (HidModule::GetInstance()->GetLoop_())
	{
		Sleep(100);
		if (HidModule::GetInstance()->last_hmd_recv_timestamp<0)
		{
			continue;
		}
		hid_timestamp_log = RVR::nowInNs();
		if ((hid_timestamp_log-HidModule::GetInstance()->last_hmd_recv_timestamp  )> 1000000000)
		{
		    ChaperOneData::GetInstance()->ResetDataBuf();
		}

	 
	}
	SetEvent(HidModule::GetInstance()->check_thread_event_);
	return 1;
}
unsigned int __stdcall HidModule::ReadHmiDataThread(LPVOID lpParameter)
{	
	HidModule::GetInstance();
	
	ChaperOneData::GetInstance();
	DriverLog("hidobj OpenHidDevice ok\n");
	
	while (HidModule::GetInstance()->GetLoop_())
	{
		while (HidModule::GetInstance()->InitHidDevice() < 0 )
		{
			if (HidModule::GetInstance()->GetLoop_()!=true)
			{
				SetEvent(HidModule::GetInstance()->read_thread_event_);
				return 0;
			}
			Sleep(HidErrorSleepTime);
		}

		while (HidModule::GetInstance()->OpenHidDevice(HidVid, HidPid) < 0)
		{
			if (HidModule::GetInstance()->GetLoop_() != true)
			{
				SetEvent(HidModule::GetInstance()->read_thread_event_);
				return 0;
			}
			Sleep(HidErrorSleepTime);
		}

		g_svrDriver.GetDpHmdDriver()->SetDpRenderFlag_(1);
		ChaperOneData::GetInstance()->ResetDataBuf();

		while (ChaperOneData::GetInstance()->RequestChaperOneData(HidType::ChaperOneRequestType::kGetStart)< 0)
		{
			if (HidModule::GetInstance()->GetLoop_() != true)
			{
				SetEvent(HidModule::GetInstance()->read_thread_event_);
				return 0;
			}
			Sleep(HidErrorSleepTime);
		}
		DriverLog("request chaperone data");
		HidModule::GetInstance()->WorkLoop();
	}
	SetEvent(HidModule::GetInstance()->read_thread_event_);
	return 0;
}
void HidModule::LogOutHidBuf(unsigned char* buf) 
{
	DriverLog("hid data start");
	for (int i = 0; i < 8; i++)
	{

		DriverLog("%0x %0x %0x %0x %0x %0x %0x %0x",
			buf[i * 8 + 0], buf[i * 8 + 1], buf[i * 8 + 2], buf[i * 8 + 3], buf[i * 8 + 4],
			buf[i * 8 + 5], buf[i * 8 + 6], buf[i * 8 + 7]);

	}
	DriverLog("hid data end");
}
void HidModule::WorkLoop() 
{
	while (HidModule::GetInstance()->GetLoop_())
	{
		unsigned char buf[256] = { 0 };
		size_t bufsize = HidBufLen;
		int ret = HidModule::GetInstance()->ReadData(buf, bufsize);

		if (ret < 0)
		{
			DriverLog("hid error");
			g_svrDriver.GetDpHmdDriver()->SetDpRenderFlag_(0);

			RVR::RVRPoseHmdData hmd_pose = { 0 };
			hmd_pose.valid = false;
			g_svrDriver.AddHmdPose(&hmd_pose, true);
			RVR::RVRControllerData controller_pose = { 0 };
			controller_pose.connectionState = RVR::RVRControllerConnectionState::kDisconnected;
			HidType::ButtonStateGather button_state = {0};
			g_svrDriver.AddControllerPose((uint32_t)ControllerIndex::kLeftController, &controller_pose, button_state);
			g_svrDriver.AddControllerPose((uint32_t)ControllerIndex::kRightController, &controller_pose, button_state);
			ret = HidModule::GetInstance()->CloseHidDevice();
			Sleep(HidErrorSleepTime);
			break;
		}
		if (ret != HidBufLen)
		{
			continue;
		}
		if (gLog)
		{
			HidModule::GetInstance()->LogOutHidBuf(buf);
		}
		HidType::HidCommon hidcommon = { 0 };
		memmove(&hidcommon, buf + 1, HidDataLen);
		HidModule::GetInstance()->OperateHidDate(hidcommon);
	}

}
 
int HidModule::OperateHmdData(HidType::HidCommon hidcommon) 
{
	float rotation[4] = { 0 };
	float position[3] = { 0 };
	float velocity[3] = { 0 };
	float acceleration[3] = { 0 };
	float angularVelocity[3] = { 0 };
	int64_t hid_timestamp = 0;
	HidType::HidStreamingData hid = { 0 };
	memmove((void*)&hid, (void*)&hidcommon, HidDataLen);
	for (int i = 0; i < 4; i++)
	{
		float tmp;
		char* pf = (char*)&tmp;
		char* pc = hid.transData.rotation.data + i * 4;
		*((char*)(pf)) = *(pc);
		*((char*)(pf + 1)) = *(pc + 1);
		*((char*)(pf + 2)) = *(pc + 2);
		*((char*)(pf + 3)) = *(pc + 3);
		rotation[i] = (float)tmp;
	}
	for (int i = 0; i < 3; i++)
	{
		float tmp;
		char* pf = (char*)&tmp;
		char* pc = hid.transData.position.data + i * 4;
		*((char*)(pf)) = *(pc);
		*((char*)(pf + 1)) = *(pc + 1);
		*((char*)(pf + 2)) = *(pc + 2);
		*((char*)(pf + 3)) = *(pc + 3);
		position[i] = (float)tmp;
	}
	for (int i = 0; i < 3; i++)
	{
		short d = hid.transData.velocity.data[0 + 2 * i];
		d = (d & 0x00ff) << 8;
		short d1 = hid.transData.velocity.data[1 + 2 * i];
		d1 = (d1 & 0x00ff);
		short tmp = d + d1;
		velocity[i] = (float)tmp / 1000.000f;

		d = hid.transData.acceleration.data[0 + 2 * i];
		d = (d & 0x00ff) << 8;
		d1 = hid.transData.acceleration.data[1 + 2 * i];
		d1 = (d1 & 0x00ff);
		tmp = d + d1;
		acceleration[i] = (float)tmp / 100.000f;


		d = hid.transData.vecAngularVelocity.data[0 + 2 * i];
		d = (d & 0x00ff) << 8;
		d1 = hid.transData.vecAngularVelocity.data[1 + 2 * i];
		d1 = (d1 & 0x00ff);
		tmp = d + d1;
		angularVelocity[i] = (float)tmp / 100.000f;
	}

	char time[8];
	for (int i = 0; i < 8; i++)
	{
		time[7 - i] = hid.transData.timestamp[i];
	}
	memmove(&hid_timestamp, time, sizeof(char) * 8);
	
	RVR::RVRPoseHmdData hmd_pose = { 0 };
	hmd_pose.valid = true;
	hmd_pose.position.x = position[0];
	hmd_pose.position.y = position[1];
	hmd_pose.position.z = position[2];

	hmd_pose.rotation.x = rotation[0];
	hmd_pose.rotation.y = rotation[1];
	hmd_pose.rotation.z = rotation[2];
	hmd_pose.rotation.w = rotation[3];
	 
	hmd_pose.linearVelocity.x = velocity[0];
	hmd_pose.linearVelocity.y = velocity[1];
	hmd_pose.linearVelocity.z = velocity[2];

	hmd_pose.linearAcceleration.x = acceleration[0];
	hmd_pose.linearAcceleration.y = acceleration[1];
	hmd_pose.linearAcceleration.z = acceleration[2];
 
	hmd_pose.angularVelocity.x= angularVelocity[0];
	hmd_pose.angularVelocity.y = angularVelocity[1];
	hmd_pose.angularVelocity.z = angularVelocity[2];
	hmd_pose.poseTimeStamp = hid_timestamp;
	hmd_pose.poseRecvTime = RVR::nowInNs();
	
	g_svrDriver.AddHmdPose(&hmd_pose,true);
	OperateIpd(hidcommon);
	return 1;
}
int HidModule::OperateControllerData(HidType::HidCommon hidcommon) 
{
	float rotation[4] = { 0 };
	float position[3] = { 0 };
	float velocity[3] = { 0 };
	float acceleration[3] = { 0 };
	float angularVelocity[3] = { 0 };
	int64_t hid_timestamp = 0;
	ControllerIndex controller_inedx =ControllerIndex::kUnknow;
	HidType::HidStreamingData hid = { 0 };
	memmove((void*)&hid, (void*)&hidcommon, HidDataLen);
	for (int i = 0; i < 4; i++)
	{
		float tmp;
		char* pf = (char*)&tmp;
		char* pc = hid.transData.rotation.data + i * 4;
		*((char*)(pf)) = *(pc);
		*((char*)(pf + 1)) = *(pc + 1);
		*((char*)(pf + 2)) = *(pc + 2);
		*((char*)(pf + 3)) = *(pc + 3);
		rotation[i] = (float)tmp;
	}
	for (int i = 0; i < 3; i++)
	{
		float tmp;
		char* pf = (char*)&tmp;
		char* pc = hid.transData.position.data + i * 4;
		*((char*)(pf)) = *(pc);
		*((char*)(pf + 1)) = *(pc + 1);
		*((char*)(pf + 2)) = *(pc + 2);
		*((char*)(pf + 3)) = *(pc + 3);
		position[i] = (float)tmp;
	}
	for (int i = 0; i < 3; i++)
	{
		short d = hid.transData.velocity.data[0 + 2 * i];
		d = (d & 0x00ff) << 8;
		short d1 = hid.transData.velocity.data[1 + 2 * i];
		d1 = (d1 & 0x00ff);
		short tmp = d + d1;
		velocity[i] = (float)tmp / 1000.000f;

		d = hid.transData.acceleration.data[0 + 2 * i];
		d = (d & 0x00ff) << 8;
		d1 = hid.transData.acceleration.data[1 + 2 * i];
		d1 = (d1 & 0x00ff);
		tmp = d + d1;
		acceleration[i] = (float)tmp / 100.000f;


		d = hid.transData.vecAngularVelocity.data[0 + 2 * i];
		d = (d & 0x00ff) << 8;
		d1 = hid.transData.vecAngularVelocity.data[1 + 2 * i];
		d1 = (d1 & 0x00ff);
		tmp = d + d1;
		angularVelocity[i] = (float)tmp / 100.000f;
	}

	char time[8];
	for (int i = 0; i < 8; i++)
	{
		time[7 - i] = hid.transData.timestamp[i];
	}
	memmove(&hid_timestamp, time, sizeof(char) * 8);

	RVR::RVRControllerData controller_pose = { 0 };
	if ((HidType::MessageType)hid.devState.type==HidType::MessageType::kLeftControllerData)
	{
		controller_inedx =ControllerIndex::kLeftController;
	}else if ((HidType::MessageType)hid.devState.type==HidType::MessageType::kRightControllerData)
	{
		controller_inedx = ControllerIndex::kRightController;
	}
	else
	{
		return 0; 
	}
	controller_pose.position.x = position[0];
	controller_pose.position.y = position[1];
	controller_pose.position.z = position[2];

	controller_pose.rotation.x = rotation[0];
	controller_pose.rotation.y = rotation[1];
	controller_pose.rotation.z = rotation[2];
	controller_pose.rotation.w = rotation[3];

	controller_pose.vecVelocity.x = velocity[0];
	controller_pose.vecVelocity.y = velocity[1];
	controller_pose.vecVelocity.z = velocity[2];

	controller_pose.vecAcceleration.x = acceleration[0];
	controller_pose.vecAcceleration.y = acceleration[1];
	controller_pose.vecAcceleration.z = acceleration[2];

	controller_pose.vecAngularVelocity.x = angularVelocity[0];
	controller_pose.vecAngularVelocity.y = angularVelocity[1];
	controller_pose.vecAngularVelocity.z = angularVelocity[2];
	controller_pose.timestamp = hid_timestamp;
	controller_pose.recvTimeStampNs = RVR::nowInNs();

	controller_pose.connectionState = (RVR::RVRControllerConnectionState)(HidType::TransControllerConnectionState)hid.devState.connection_state;
	HidType::ButtonStateGather button_state;
	memmove(&button_state, hid.transData.button_state, sizeof(HidType::ButtonStateGather));
	char button_state_buf[2] = { 0 };
	memmove(button_state_buf, &button_state.button_state, sizeof(short));
	short d = button_state_buf[0];
	d = (d & 0x00ff) << 8;
	short d1 = button_state_buf[1];
	d1 = (d1 & 0x00ff);
	d = d + d1;
	memmove(&button_state.button_state, &d, sizeof(short));
	if (g_svrDriver.GetStreamingHmdDriver()->GetIsAdded_()==false)
	{
		g_svrDriver.AddControllerPose((uint32_t)controller_inedx, &controller_pose, button_state);
	}
	
	return 1;
}
int HidModule::OperateIpd(HidType::HidCommon hidcommon) 
{
	HidType::HidStreamingData hid = { 0 };
	memmove((void*)&hid, (void*)&hidcommon, HidDataLen);
	float ipd_data = hid.transData.ipd;
	ipd_data = (ipd_data + 578) / 10000;
	float ipd = g_svrDriver.GetDpHmdDriver()->GetIpd_();
	if (!FLT_EQUAL(ipd_data, g_svrDriver.GetDpHmdDriver()->GetIpd_()))
	{
		DriverLog("driver ipd change: IPD: %f\n", g_svrDriver.GetDpHmdDriver()->GetIpd_());
		PropertyContainerHandle_t ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(g_svrDriver.GetDpHmdDriver()->GetObjectId_());

		g_svrDriver.GetDpHmdDriver()->m_eyeToHeadLeft.m[0][3] = -ipd_data / 2.0f;
		g_svrDriver.GetDpHmdDriver()->m_eyeToHeadRight.m[0][3] = ipd_data / 2.0f;
		//vr::VRServerDriverHost()->SetDisplayEyeToHead(hmd_device_.GetObjectId_(), hmd_device_.m_eyeToHeadLeft, hmd_device_.m_eyeToHeadRight);
		g_svrDriver.GetDpHmdDriver()->SetIpd_(ipd_data);
		vr::VRProperties()->SetFloatProperty(ulPropertyContainer, Prop_UserIpdMeters_Float, g_svrDriver.GetDpHmdDriver()->GetIpd_());


		vr::HmdRect2_t m_eyeFoVLeft;
		vr::HmdRect2_t m_eyeFoVRight;
		g_svrDriver.GetDpHmdDriver()->GetProjectionRaw(vr::EVREye::Eye_Left,
			&m_eyeFoVLeft.vTopLeft.v[0],
			&m_eyeFoVLeft.vBottomRight.v[0],
			&m_eyeFoVLeft.vTopLeft.v[1],
			&m_eyeFoVLeft.vBottomRight.v[1]);
		g_svrDriver.GetDpHmdDriver()->GetProjectionRaw(vr::EVREye::Eye_Right,
			&m_eyeFoVRight.vTopLeft.v[0],
			&m_eyeFoVRight.vBottomRight.v[0],
			&m_eyeFoVRight.vTopLeft.v[1],
			&m_eyeFoVRight.vBottomRight.v[1]);
		if (ipd_data< 0.060 || ipd_data>0.7)
		{
			ipd_data = 0.64;
		}
		if (ipd_data > 0.065)
		{
			float halfFovInRadians = 95 * DEGREES_TO_RADIANS * 0.5f;
			float tangent = tan(halfFovInRadians);

			m_eyeFoVLeft.vTopLeft.v[0] = m_eyeFoVLeft.vTopLeft.v[1] = -tangent;
			m_eyeFoVLeft.vBottomRight.v[0] = m_eyeFoVLeft.vBottomRight.v[1] = tangent;

			m_eyeFoVRight.vTopLeft.v[0] = m_eyeFoVRight.vTopLeft.v[1] = -tangent;
			m_eyeFoVRight.vBottomRight.v[0] = m_eyeFoVRight.vBottomRight.v[1] = tangent;
		}
		else if (0.06 < ipd_data < 0.065)
		{
			float halfFovInRadians = 95 * DEGREES_TO_RADIANS * 0.5f;
			float tangent = tan(halfFovInRadians);

			m_eyeFoVLeft.vTopLeft.v[0] = m_eyeFoVLeft.vTopLeft.v[1] = -tangent;
			m_eyeFoVLeft.vBottomRight.v[0] = m_eyeFoVLeft.vBottomRight.v[1] = tangent;

			m_eyeFoVRight.vTopLeft.v[0] = m_eyeFoVRight.vTopLeft.v[1] = -tangent;
			m_eyeFoVRight.vBottomRight.v[0] = m_eyeFoVRight.vBottomRight.v[1] = tangent;
		}
		else if (0.060 > ipd_data)
		{
			float halfFovInRadians = 95 * DEGREES_TO_RADIANS * 0.5f;
			float tangent = tan(halfFovInRadians);

			m_eyeFoVLeft.vTopLeft.v[0] = m_eyeFoVLeft.vTopLeft.v[1] = -tangent;
			m_eyeFoVLeft.vBottomRight.v[0] = m_eyeFoVLeft.vBottomRight.v[1] = tangent;

			m_eyeFoVRight.vTopLeft.v[0] = m_eyeFoVRight.vTopLeft.v[1] = -tangent;
			m_eyeFoVRight.vBottomRight.v[0] = m_eyeFoVRight.vBottomRight.v[1] = tangent;
		}

		//vr::VRServerDriverHost()->SetDisplayProjectionRaw(ulPropertyContainer, m_eyeFoVLeft, m_eyeFoVRight);
		vr::VRServerDriverHost()->VendorSpecificEvent(ulPropertyContainer, vr::VREvent_LensDistortionChanged, {}, 0);
		VREvent_Ipd_t data;
		data.ipdMeters = ipd_data;
		vr::VRServerDriverHost()->VendorSpecificEvent(ulPropertyContainer, vr::VREvent_IpdChanged, (vr::VREvent_Data_t&)data, 0);
	}
	return 1;
}
int HidModule::OperateDistortionOffser(HidType::HidCommon hidcommon) 
{
	HidType::HidAxsiOffsetData hid = { 0 };
	memmove((void*)&hid, (void*)&hidcommon, sizeof(HidType::HidAxsiOffsetData));
	
	if (hid.devState.type == 3)
	{
		float offset[12] = { 0 };
		memmove(offset, &hid.axisOffset, sizeof(float) * 12);
		if ((fabs(offset[4] - 0.0f) < 0.0001) && (fabs(offset[5] - 0.0f) < 0.0001) &&
			(fabs(offset[6] - 0.0f) < 0.0001) && (fabs(offset[7] - 0.0f) < 0.0001))
		{
			g_svrDriver.GetDpHmdDriver()->SetHmdType(Hmd_TYPE::EYE);
		}
		else
		{
			g_svrDriver.GetDpHmdDriver()->SetHmdType(Hmd_TYPE::PRO);
		}
		if (g_svrDriver.GetDpHmdDriver()->GetHmdType() == Hmd_TYPE::EYE)
		{
			g_svrDriver.GetDpHmdDriver()->SetOffsetIpd_(0.058, offset[0], offset[1], offset[2], offset[3]);
			g_svrDriver.GetDpHmdDriver()->SetOffsetIpd_(0.063, offset[0], offset[1], offset[2], offset[3]);
			g_svrDriver.GetDpHmdDriver()->SetOffsetIpd_(0.069, offset[0], offset[1], offset[2], offset[3]);
		}
		else if (g_svrDriver.GetDpHmdDriver()->GetHmdType() == Hmd_TYPE::PRO)
		{
			g_svrDriver.GetDpHmdDriver()->SetOffsetIpd_(0.058, offset[0], offset[1], offset[2], offset[3]);
			g_svrDriver.GetDpHmdDriver()->SetOffsetIpd_(0.063, offset[4], offset[5], offset[6], offset[7]);
			g_svrDriver.GetDpHmdDriver()->SetOffsetIpd_(0.069, offset[8], offset[9], offset[10], offset[11]);
		}
		if (gLog)
		{
			DriverLog("set offset ipd=%f,offset0=%f,offset1=%f,offset2=%f,offset3=%f,offset4=%f,offset5=%f,offset6=%f, \
            offset7=%f, offset8=%f, offset9=%f, offset10=%f, offset11=%f", g_svrDriver.GetDpHmdDriver()->GetIpd_(),
				offset[0], offset[1], offset[2], offset[3],
				offset[4], offset[5], offset[6], offset[7],
				offset[8], offset[9], offset[10], offset[11]);
		}


	}
	return 1;
}