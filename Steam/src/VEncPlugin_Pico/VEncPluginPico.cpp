#include "RtpPacket.h"
#include "VEncPluginPico.h"
#include "GlobalDLLContext.h"
#include "EncoderComponent/AMFComponents/VideoEncoderVCE.h"
#include "EncoderComponent/VideoEncoderFrameConfig.h"
#include "EncoderComponent/NVENCComponents/VideoEncoderNVENC.h"
#include "./CheckGraphicCard.h"
#include "./pxrTool/TimeTool.h"
#include "./pxrTool/config_reader.h"
#include "./pxrTool/filetool.h"
#include "./pxrTool/stringtool.h"
#include "RtpQualityHelper.h" 

ConfigReader gConfig;
extern SendIndex gSendIndex;
#define AMD_ENCODER 0
#define NV_ENCODER 1
#define NO_ENCODER 2
extern int gEncodeType;
extern int gRtpPloadTpye;
int gEncoderDeviceType = AMD_ENCODER;
int gEcoderIndex = 0;
extern std::string gDstIp;
extern bool gLog;
 
extern bool leftgDstFlag;
extern bool rightgDstFlag;
extern std::string gDstIp;
extern RVR::RVRPoseHmdData gRenderPoseList[POSELISTSIZE];
extern uint64_t gRenderPoseIndex;
bool gIsDebug = false;
int gInsertIdr = 0;
bool gChangeGop = false;
bool gChagneBitRate = false;
bool gSavePicture = false;
bool gSaveVideo = false;
bool gLog = false;
bool gDecrease = false;
bool gIncrease = false;
int gBitRateControl = 0;
int  gCurrentBitRate = 0;
int gConfigRateChange = 0;
HRESULT VEncPluginPico::Initialize(RVR::VEncConfig* encConfig)
{
	GLOBAL_DLL_CONTEXT_LOG()->LogTrace(__FUNCTION__);
	//OutputDebugString(L"encoder->Initialize begin");
	string configPath;
	char driverpath[1024] = { 0 };
	GetSelfModulePath(driverpath);
	configPath = driverpath;
	deletesub(configPath, "VEncPlugin.dll", strlen("VEncPlugin.dll"));
	configPath += "RVRPlugin.ini";
	GLOBAL_DLL_CONTEXT_LOG()->LogAlways(configPath);
	gConfig.ReadConfig(configPath);
	gLog = gConfig.GetLog() == 1 ? true:false;
	mIndex = gEcoderIndex;
	if (gEcoderIndex == 0)
	{
		 
		mHThreadEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("rtpthread  Event 1"));
	}
	else 
	{
		mHThreadEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("rtpthread Event 2"));
	}
	VideoEncoderConfig config;
	memset(&config, 0, sizeof(VideoEncoderConfig));
	memcpy_s(&config, sizeof(VideoEncoderConfig), encConfig, sizeof(RVR::VEncConfig));
	
	if (IfNVIDIACard())
	{
		gEncoderDeviceType = NV_ENCODER ;
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("=========NVIDIA  Encoder========");
	}
	else if(IfAMDCard())
	{
		gEncoderDeviceType = AMD_ENCODER;
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("=========AMD  Encoder========");
	}
	else
	{
		gEncoderDeviceType = NO_ENCODER;
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("=========NO  Encoder========");
	}

	if (gEncoderDeviceType == AMD_ENCODER)
	{
		encoder = new VideoEncoderVCE();		 
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("=========NEW  AMD  Encoder========");
	}
	else if(gEncoderDeviceType == NV_ENCODER)
	{
		encoder = new VideoEncoderNVENC();		
		//encoder = new VideoEncoderVCE();
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("=========NEW NVIDIA  Encoder========");
	}
	else
	{
		encoder = nullptr;
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("=========Do Not Find  AMD or NVIDIA  Encoder========");
		return -1;
	}

	//20220121
	if (encoder != nullptr)
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("=========  Encoder is  not  nullptr ========");
		encoder->mIndex = gEcoderIndex;
		encoder->CreateOutFrameBufferAndInitParts();
		encoder->Initialize(config);
		if ((gEncoderDeviceType == AMD_ENCODER) && (encoder->init_success_ == false))
		{
			gEncoderDeviceType = NV_ENCODER;
			encoder = new VideoEncoderNVENC();
			encoder->Initialize(config);
		}

		if (encoder->init_success_)
		{
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways("encoder build");
		}
		else
		{
			if (gEncoderDeviceType == NV_ENCODER)
			{
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways("encoder cuda test");
				//20220121
				encoder = nullptr;
				delete encoder;
				return -1;
			}
		}

		if (encoder != nullptr)
		{
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways("=========	RTP	Instance========");
			RtpQualityHelper::GetInstance();
			RtpPacket::GetInstance(mIndex);
			RtpPacket::GetInstance(mIndex)->SetConfig(rtpcongtype::fps, gConfig.GetFps());
			TcpPacket::GetInstance(mIndex);

			if (mIndex == 0)
			{
				RtpQualityHelper::GetInstance()->Init();
				RtpQualityHelper::GetInstance()->SetBitRate(gConfig.GetAverageBitRateValue(), gConfig.GetMaxBitRateValue());				

			}
			
			std::string msg = "gDstIp:" + gDstIp;
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);

			//if (gConfig.GetRtcOrBulkMode_()==0)
			{
				ResetEvent(mHThreadEvent);
				HANDLE ret = (HANDLE)_beginthreadex(NULL, 0, &PacketAndSendThread, this, 0, &mPacketAndSendThreadId);

				WaitForSingleObject(mHThreadEvent, INFINITE);
				BOOL set_ret = SetThreadPriority(ret, THREAD_PRIORITY_TIME_CRITICAL);
				if (set_ret != TRUE)
				{
					std::string msg = "SetThreadPriority errror";
					GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				}
				CloseHandle(ret);
				encoder->SetRtpThreadId(mPacketAndSendThreadId);
				if (mIndex == 0)
				{
					gSendIndex.left_send_thread_id_ = mPacketAndSendThreadId;
				}
				else
				{
					gSendIndex.right_send_thread_id_ = mPacketAndSendThreadId;
				}
			}
			
			gEcoderIndex++;
			msg = "encoder index=" + to_string(gEcoderIndex);
			GLOBAL_DLL_CONTEXT_LOG()->LogTrace(msg);
			if (mIndex == 0)
			{
				_beginthreadex(NULL, 0, &DebugThread, this, 0, NULL);
			}
			mStartTimeUs = GetTimestampUs();
		}
	}
	return 0;
}

HRESULT VEncPluginPico::StopLoop() 
{
	ResetEvent(encoder->mHThreadEvent);
	encoder->mEncoderRun = false;
	return 1;
}

HRESULT VEncPluginPico::Uninitialize()
{
	GLOBAL_DLL_CONTEXT_LOG()->LogTrace(__FUNCTION__);
	
	WaitForSingleObject(encoder->mHThreadEvent, 1500);
	if (nullptr != encoder)
	{
		encoder->Shutdown();
		delete encoder;
		encoder = nullptr;
	}

	return 0;
}

HRESULT VEncPluginPico::QueueBuffer(ID3D11Texture2D* inputBuffer, RVR::VEncFrameConfig* frameConfig)
{
	GLOBAL_DLL_CONTEXT_LOG()->LogTrace(__FUNCTION__);

	if (nullptr != encoder)
	{
		VideoEncoderFrameConfig config;
		memcpy_s(&config, sizeof(VideoEncoderFrameConfig), frameConfig, sizeof(RVR::VEncFrameConfig));
		config.originPointer = static_cast<void*>(frameConfig);
	    
		if (gDecrease||((config.flags& RVR::VENC_BITRATE_UPDATE)&&(config.avgBitRate<0)))
		{
			RtpQualityHelper::GetInstance()->DecreaseBitRate(2);
			int averageRate = 0;
			int maxRate = 0;
			RtpQualityHelper::GetInstance()->GetCurrentRate(averageRate, maxRate);
			string msg = "debug decrease rate," + to_string(averageRate) + "," + to_string(maxRate);
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
			gDecrease = false;
		}
		if (gBitRateControl>0)
		{
			if (gCurrentBitRate== MAXBITRATE|| gCurrentBitRate==MINBITRATE)
			{
				string msg = "current bitrate," + to_string(gCurrentBitRate)  ;
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
			} 
			else
			{
				gBitRateControl = gBitRateControl - 1;
				gCurrentBitRate = gConfig.GetBitRateControl() + gConfig.GetAverageBitRateValue();
				if (gCurrentBitRate > MAXBITRATE)
				{
					gCurrentBitRate = MAXBITRATE;
				}
				if (gCurrentBitRate < MINBITRATE)
				{
					gCurrentBitRate = MINBITRATE;
				}
				config.avgBitRate = gCurrentBitRate;
				config.maxBitRate = gCurrentBitRate + gCurrentBitRate / 5;
				config.flags = config.flags | RVR::VENC_BITRATE_UPDATE_BY_USER;
			}
			
		}
		if (gConfigRateChange > 0)
		{
			gConfigRateChange = gConfigRateChange - 1;
			config.avgBitRate = gConfig.GetBitRate();
			config.maxBitRate = gConfig.GetMaxBitRateValue();
			config.flags = config.flags | RVR::VENC_BITRATE_UPDATE_BY_CONFIG;

		}
		encoder->Transmit(inputBuffer, &config);
	}

	return 0;
}

HRESULT VEncPluginPico::Flush()
{
	GLOBAL_DLL_CONTEXT_LOG()->LogTrace(__FUNCTION__);

	if (nullptr != encoder)
	{
		encoder->Flush();
	}

	return 0;
}

 

VEncPluginPico::VEncPluginPico()
	:instance_ptr_str_(std::to_string(reinterpret_cast<int64_t>(this)))
	, encoder(nullptr)
{
	GLOBAL_DLL_CONTEXT_LOG()->LogAlways("VEncPlugin Encode framework: AMF. Instance Point:" + instance_ptr_str_);
}

VEncPluginPico::~VEncPluginPico()
{
	//Explicitly call this in case of missing from top level
	this->VEncPluginPico::Uninitialize();
}
 
//
//HRESULT VEncPluginPicoNullDllForQualcomm::Initialize(RVR::VEncConfig * encConfig)
//{
//	return E_NOTIMPL;
//}
//
//HRESULT VEncPluginPicoNullDllForQualcomm::Uninitialize()
//{
//	return E_NOTIMPL;
//}
//
//HRESULT VEncPluginPicoNullDllForQualcomm::QueueBuffer(ID3D11Texture2D * inputBuffer, RVR::VEncFrameConfig * frameConfig)
//{
//	return E_NOTIMPL;
//}
//
//HRESULT VEncPluginPicoNullDllForQualcomm::Flush()
//{
//	return E_NOTIMPL;
//}
//
//VEncPluginPicoNullDllForQualcomm::VEncPluginPicoNullDllForQualcomm()
//{
//}
//
//VEncPluginPicoNullDllForQualcomm::~VEncPluginPicoNullDllForQualcomm()
//{
//}

unsigned int __stdcall VEncPluginPico::DebugThread(LPVOID lpParameter) 
{
	VEncPluginPico* EncPluginObj = (VEncPluginPico*)lpParameter;
	VideoEncoder* EncoderObj = (VideoEncoder*)(((VEncPluginPico*)lpParameter)->encoder);
	while (EncoderObj->mEncoderRun)
	{
		gConfig.GetAutoRate();
		gConfig.GetFec();
		gConfig.GetRtt();
		gConfig.GetRtpAck();
		gConfig.GetBitRate();
		gConfig.GetTcp();
		gConfig.GetRtcOrBulkModeFromFile_();
		if (((0x8000 & GetAsyncKeyState(VK_SHIFT)) != 0)&& ((0x8000 & GetAsyncKeyState('1')) != 0) && ((0x8000 & GetAsyncKeyState('O')) != 0))
		{
			gIsDebug = true;
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways("venc debug open");
		}

		if (((0x8000 & GetAsyncKeyState(VK_SHIFT)) != 0)&& ((0x8000 & GetAsyncKeyState('0')) != 0))
		{
			gIsDebug = false;
			gInsertIdr = 0;
			gChangeGop = false;
			gChagneBitRate = false;
			gSavePicture = false;
			gSaveVideo = false;
			gLog = false;
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways("venc debug close");
		}
		if (((0x8000 & GetAsyncKeyState(VK_CONTROL)) != 0) && (((0x8000 & GetAsyncKeyState(VK_OEM_PLUS)) != 0)|| ((0x8000 & GetAsyncKeyState(VK_ADD)) != 0)))
		{
			gBitRateControl = 2;
			gConfig.SetBitRateControl(gConfig.GetBitRateControl() + BITRATECHANGE);
			
		}
		if (((0x8000 & GetAsyncKeyState(VK_CONTROL)) != 0) && (((0x8000 & GetAsyncKeyState(VK_OEM_MINUS)) != 0)|| ((0x8000 & GetAsyncKeyState(VK_SUBTRACT)) != 0)))
		{
			gBitRateControl = 2;
			gConfig.SetBitRateControl(gConfig.GetBitRateControl() - BITRATECHANGE);
		}
		 
		if (gIsDebug)
		{
			if ((0x8000 & GetAsyncKeyState('I')) != 0)
            {
				gInsertIdr = 4;
				 
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways("gInsertIdr");
				
			}

			if ((0x8000 & GetAsyncKeyState('G')) != 0)
			{
				gChangeGop = true;
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways("gChangeGop");
			}
			if ((0x8000 & GetAsyncKeyState('D')&& (0x8000 & GetAsyncKeyState('R')) != 0))
			{
				gDecrease = true;
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways("gDecrease");
			}
			if ((0x8000 & GetAsyncKeyState('H') && (0x8000 & GetAsyncKeyState('R')) != 0))
			{
				gIncrease = true;
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways("gIncrease");
				RtpQualityHelper::GetInstance()->ForceMaxBitRate(gConfig.GetAverageBitRateValue(), gConfig.GetMaxBitRateValue());
				gIncrease = false;
			}
			if ((0x8000 & GetAsyncKeyState('B')) != 0)
			{
				gChagneBitRate = true;
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways("gChagneBitRate");
			}
			if ((0x8000 & GetAsyncKeyState('V')) != 0 && (0x8000 & GetAsyncKeyState('O')) != 0)
			{
			
				gSaveVideo = true;
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways("gSaveVideo open");
			}
			if ((0x8000 & GetAsyncKeyState('L')) != 0 && (0x8000 & GetAsyncKeyState('O')) != 0)
			{

				gLog = true;
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways("gLog open");
			}
			if ((0x8000 & GetAsyncKeyState('L')) != 0 && (0x8000 & GetAsyncKeyState('C')) != 0)
			{

				gLog = false;
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways("gLog close");
			}
			if ((0x8000 & GetAsyncKeyState('V')) != 0 && (0x8000 & GetAsyncKeyState('C')) != 0)
			{
				gSaveVideo = false;
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways("gSaveVideo close");
			}

			if ((0x8000 & GetAsyncKeyState('P')) != 0 && (0x8000 & GetAsyncKeyState('O')) != 0)
			{
				gSavePicture = true;
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways("gSavePicture open");
			}
			if ((0x8000 & GetAsyncKeyState('P')) != 0 && (0x8000 & GetAsyncKeyState('C')) != 0)
			{
				gSavePicture = false;
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways("gSavePicture close");
			}
			if (gInsertIdr>0||gChagneBitRate||gChangeGop|| gBitRateControl)
			{
				Sleep(500);
			}
		}
		Sleep(5);
	}
	return 1;
}

unsigned int __stdcall VEncPluginPico::PacketAndSendThread(LPVOID lpParameter)
{
	 
	VEncPluginPico* EncPluginObj = (VEncPluginPico*)lpParameter;
	VideoEncoder* EncoderObj = (VideoEncoder*)(((VEncPluginPico*)lpParameter)->encoder);
	uint64_t bufferIndex = 0;
	uint64_t poseIndex = 0;
	int eyeIndex = 0xff;
	 
	SetEvent(EncPluginObj->mHThreadEvent);
	 
	AddPoseData adddata;
	AddQuaternion quaternion;
	AddVector3 addvector3;
	uint64_t costall = 0;
	double timecount = 0.0;
	string msgLog;	
	uint64_t last_out_frame = 0;
	uint64_t last_buf_index=0;
	while (EncoderObj->mEncoderRun)
	{	
		if (last_out_frame== EncoderObj->out_frame_index_)
		{
			//sleep_micro_seconds(1);
			Sleep(1);
			continue;
		}
		if (EncoderObj==nullptr)
		{
			break;
		}
		 
	    bufferIndex = last_out_frame;		
		poseIndex = EncoderObj->mOutFrame[last_out_frame%OUTBUFSIZE].index;
		eyeIndex = EncoderObj->mIndex;
		if (last_buf_index !=0)
		{
			if (last_buf_index >= bufferIndex)
			{		
				string msg = "send thread index errrorloop eye index=" + to_string(eyeIndex) + "lastIndex" +
					to_string(last_buf_index) + "poseIndes" + to_string(bufferIndex);
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				continue;
			}
		}
		last_buf_index = bufferIndex;
		if (last_buf_index ==0xFFFFFFFFFFFFFFFF)
		{
			last_buf_index = 0;
		}
		
		if (gLog)
		{
			int index = gConfig.BigPicture() == 1 ? poseIndex : bufferIndex;
			string msg = "send thread eye index=" + to_string(eyeIndex) + "  poseindex=" + to_string(index)+
			"bufindex= "+to_string(bufferIndex)+	"  outlastindex="+to_string(last_out_frame);
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
		}
		
		if (EncoderObj->mIndex == 0)
		{
			if ( leftgDstFlag == true)
			{
				
				if (gConfig.GetTcpValue() == 1)
				{
					TcpPacket::GetInstance(0);
					if (TcpPacket::GetInstance(0)->InitSocket(gDstIp, gConfig.GetPortL()) == 1)
					{
						GLOBAL_DLL_CONTEXT_LOG()->LogAlways("tcp left ok");
					}
					else
					{
						GLOBAL_DLL_CONTEXT_LOG()->LogAlways("tcp left error");
					}
					TcpPacket::GetInstance(0)->SetDstIp(gDstIp);
				}
				else
				{
					RtpPacket::GetInstance(0)->InitSocket(gDstIp.c_str(), gConfig.GetPortL(), 23);
					RtpPacket::GetInstance(0)->SetDstIp(gDstIp);

				}
				leftgDstFlag = false;
			}

		}
		else
		{
			if (  rightgDstFlag == true)
			{
				RtpQualityHelper::GetInstance()->SetSocketParam(RtpPacket::GetInstance(0)->m_SocketClient,
					RtpPacket::GetInstance(1)->m_SocketClient, gDstIp, gConfig.GetPortL(), gConfig.GetPortR());
				rightgDstFlag = false;
				if (gConfig.GetTcpValue() == 1)
				{
					if (gConfig.GetTcpChannel() == 2)
					{
						TcpPacket::GetInstance(1);
						if (TcpPacket::GetInstance(1)->InitSocket(gDstIp, gConfig.GetPortR()) == 1)
						{
							GLOBAL_DLL_CONTEXT_LOG()->LogAlways("tcp right ok");
						}
						else
						{
							GLOBAL_DLL_CONTEXT_LOG()->LogAlways("tcp right error");
						}
						TcpPacket::GetInstance(1)->SetDstIp(gDstIp);
					}
					else
					{
						while (leftgDstFlag == true)//单通道传输，只用左眼通道，等待左眼连接
						{
							Sleep(1);
							GLOBAL_DLL_CONTEXT_LOG()->LogAlways("leftgDstFlag loop");
						}
						GLOBAL_DLL_CONTEXT_LOG()->LogAlways("leftgDstFlag loopout");
					}

				}
				else
				{
					RtpPacket::GetInstance(1)->InitSocket(gDstIp.c_str(), gConfig.GetPortR(), 24);
					RtpPacket::GetInstance(1)->SetDstIp(gDstIp);
				}
			}

		}
		if (EncoderObj == nullptr)
		{
			break;
		}
		RVR::RVRPoseHmdData currentPose;
		uint64_t renderCost = 0;
		if (gConfig.BigPicture() == 1)
		{
			//大图模式，poseindex 每帧增加，编码器间隔调用，bufferindex间隔一帧增加1个，
			currentPose = gRenderPoseList[poseIndex % POSELISTSIZE];
			renderCost = EncoderObj->mRenderCost[poseIndex % POSELISTSIZE];
		}
		else if (gConfig.BigPicture() == 0)
		{
			currentPose = gRenderPoseList[bufferIndex % POSELISTSIZE];
			renderCost = EncoderObj->mRenderCost[bufferIndex % POSELISTSIZE];
		}
		quaternion.w = currentPose.rotation.w;
		quaternion.x = currentPose.rotation.x;
		quaternion.y = currentPose.rotation.y;
		quaternion.z = currentPose.rotation.z;
		adddata.rotation = quaternion;

		addvector3.x = currentPose.position.x;
		addvector3.y = currentPose.position.y;
		addvector3.z = currentPose.position.z;
		adddata.position = addvector3;
		adddata.hmdPoseTimeTs = currentPose.hmdTimeStamp;
		adddata.poseTimeStamp = currentPose.poseTimeStamp;
		adddata.predictedTimeMs = currentPose.predictedTimeMs;
		int gamerender_cost = currentPose.endGameRenderStamp - currentPose.beginGameRenderStamp;
		int encode_cost = currentPose.endEncodeStamp - currentPose.beginEncodeStamp;
		int bit_rate = 0;
		int max_rate = 0;
		RtpQualityHelper::GetInstance()->GetCurrentRate(bit_rate, max_rate);
		int auto_rate = gConfig.GetAutoRateValue();
		adddata.gameRenderCost = gamerender_cost;
		adddata.encodeCost = encode_cost;
		adddata.autoRateFlag = auto_rate;
		adddata.encodeRate = bit_rate;

		/*string msg = "game rendercost:" + std::to_string((gamerender_cost) / 1000000.f) + */
		string msg = "encode cost:"+std::to_string((encode_cost) / 1000000.f);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);

		uint64_t packetsendbegin = nowInNs();
		int bufferlen = EncoderObj->mOutFrame[bufferIndex % OUTBUFSIZE].len;

		if (bufferlen == 0)
		{
			last_out_frame++;
			continue;
		}
		 
		uint64_t encodeBegin = currentPose.poseRecvTime ;
		//adddata.poseRecvTime = timeoffset;
		if (gConfig.GetRtcOrBulkMode_()==0 && currentPose.valid)
		{
			if (gConfig.GetTcpValue())
			{
				if (gConfig.GetTcpChannel() == 1)
				{

					TcpPacket::GetInstance(0)->DoPacket(EncoderObj->mOutFrame[bufferIndex % OUTBUFSIZE].buf, EncoderObj->mOutFrame[bufferIndex % OUTBUFSIZE].len, gEncodeType == 1 ? 0 : 1, 1, (uint8_t*)&adddata, sizeof(AddPoseData), (uint8_t)eyeIndex);
				}
				else
				{
					TcpPacket::GetInstance(EncoderObj->mIndex)->DoPacket(EncoderObj->mOutFrame[bufferIndex % OUTBUFSIZE].buf, EncoderObj->mOutFrame[bufferIndex % OUTBUFSIZE].len, gEncodeType == 1 ? 0 : 1, 1, (uint8_t*)&adddata, sizeof(AddPoseData));
				}
			}
			else
			{
				RtpPacket::GetInstance(EncoderObj->mIndex)->DoPacket(EncoderObj->mOutFrame[bufferIndex % OUTBUFSIZE].buf, EncoderObj->mOutFrame[bufferIndex % OUTBUFSIZE].len, gEncodeType == 1 ? 0 : 1, 1, (uint8_t*)&adddata, sizeof(AddPoseData));

			}
			EncoderObj->mOutFrame[bufferIndex % OUTBUFSIZE].len = 0;
		}
		
	

		uint64_t packetsendend = nowInNs();
	
		if (gLog)
		{
			if (EncoderObj->mIndex == 1)
			{

				string msg = "right all cost time:" + std::to_string((packetsendend  - encodeBegin) / 1000000.f);
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				costall = costall + packetsendend  - encodeBegin;
				timecount = timecount + 1.000;
				msg = "average right encode and send cost:" + std::to_string(costall / timecount / 1000000.f);
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				msg = "right sendtime time:" + std::to_string(float(packetsendend - packetsendbegin) / 1000000000.00f) + " buffer len= " + std::to_string(bufferlen);
				msg = msg + "hmdts:" + std::to_string(adddata.hmdPoseTimeTs);
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
			}
			else
			{
				string msg = "left all cost time:" + std::to_string((packetsendend  - encodeBegin) / 1000000.f);
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				costall = costall + packetsendend  - encodeBegin;
				timecount = timecount + 1.000;
				msg = "average left encode and send cost:" + std::to_string(costall / timecount / 1000000.f);
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				msg = "left sendtime time:" + std::to_string(float(packetsendend - packetsendbegin) / 1000000000.00f) + " buffer len= " + std::to_string(bufferlen);
				msg = msg + "hmdts:" + std::to_string(adddata.hmdPoseTimeTs);
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
			}
		}
		string msg_slardar ="";
		int averageRate = 0;
		int maxRate = 0;
		RtpQualityHelper::GetInstance()->GetCurrentRate(averageRate, maxRate);
		if (gConfig.BigPicture() == 1)
		{
			string msg = "output log:" + to_string(poseIndex) + "," + to_string((float)renderCost / 1000000.f) + "," +
				to_string((float)(packetsendbegin  - encodeBegin) / 1000000.f- (float)renderCost / 1000000.f - float(packetsendend - packetsendbegin) / 1000000.f) + "," + to_string(float(packetsendend - packetsendbegin) / 1000000.f) +
				+"," + to_string(averageRate) + "," + to_string(maxRate);
			if (gLog)
			{
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
			}
			msg_slardar = msg_slardar + to_string(poseIndex) + "," + to_string((float)renderCost / 1000000000.f) + "," +
				to_string((float)(packetsendbegin  - encodeBegin) / 1000000.f- (float)renderCost / 1000000.f - float(packetsendend - packetsendbegin) / 1000000.f) + "," + to_string(float(packetsendend - packetsendbegin) / 1000000.f) +
				+"," + to_string(averageRate) + "," + to_string(maxRate) ;
			
		}
		else
		{
			if (EncoderObj->mIndex == 1)
			{
				string msg = "output log:" + to_string(poseIndex) + "," + to_string((float)renderCost / 1000000.f) + "," +
					to_string((float)(packetsendbegin  - encodeBegin) / 1000000.f) + "," + to_string(float(packetsendend - packetsendbegin) / 1000000.f) +
					"," + to_string(averageRate) + "," + to_string(maxRate);
				if (gLog)
				{
					GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				}
			}

			msg_slardar = msg_slardar + to_string(poseIndex) + "," + to_string((float)renderCost / 1000.f) + "," +
				to_string((float)(packetsendbegin / 1000 - encodeBegin) / 1000.f) + "," + to_string(float(packetsendend - packetsendbegin) / 1000000.f) +
				+"," + to_string(averageRate) + "," + to_string(maxRate) ;

		}

		GLOBAL_DLL_CONTEXT_LOG()->LogSlardar(msg_slardar);
		last_out_frame++;


		{
			uint64_t ts = GetTimestampUs();
			if (EncPluginObj->mLastStatisticsTimestamp == 0)
			{
				EncPluginObj->mLastStatisticsTimestamp = GetTimestampUs();
			}
			EncPluginObj->mFrameCount++;
			EncPluginObj->mFrameCountSecond++;
			if ((ts - EncPluginObj->mLastStatisticsTimestamp )> 1000000)
			{
				EncPluginObj->mStatisticsCount++;
				EncPluginObj->mLastStatisticsTimestamp = GetTimestampUs();
				string msg = "frame count in second:" + to_string(EncPluginObj->mFrameCountSecond) +
					"   average frame in second: " + to_string(float(EncPluginObj->mFrameCount/ EncPluginObj->mStatisticsCount));
				EncPluginObj->mFrameCountSecond = 0;
				if (gLog)
				{
					GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				}
			}
		}

	}
	SetEvent(EncoderObj->mHThreadEvent);
	return 1;
}