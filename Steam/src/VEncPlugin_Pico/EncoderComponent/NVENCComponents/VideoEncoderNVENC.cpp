#include "../RtpPacket.h"
#include "VideoEncoderNVENC.h"
#include "../../GlobalDLLContext.h"
#include "../RVRPlugin/IEncOutputSink.h"
#include "../VideoEncoderFrameConfig.h"

#include "d3d11_4.h"
#include "../D3DHelper.h"

#include "../pxrTool/TimeTool.h"
#include "../pxrTool/config_reader.h"
#include "../pxrTool/filetool.h"
#include "../pxrTool/stringtool.h"
#include "../pxrTool/TimeTool.h"
#include "../pxrTool/DebugHelper.h"
#include "../RtpQualityHelper.h"
#include <process.h>
#pragma comment( lib, "d3d11.lib" )
uint64_t LeftEncodeBegin[50];
int EncodeTsIndex = 0;
extern int gRgbType;
extern int gEcoderIndex ;
extern ConfigReader gConfig;
int gindex = 0;
extern std::string gDstIp;
extern int gInsertIdr;
extern bool gChangeGop;
extern bool gChagneBitRate;
extern bool gSavePicture  ;
extern bool gSaveVideo ;

extern int gEncodeType;
extern int gRtpPloadTpye;
extern bool gLog;
extern SendIndex gSendIndex;
extern RVR::RVRPoseHmdData gRenderPoseList[POSELISTSIZE];
extern uint64_t gRenderPoseIndex;
VideoEncoderNVENC::VideoEncoderNVENC() :  mSvrFrameConfigIndex(0)
{
	
}

VideoEncoderNVENC::~VideoEncoderNVENC()
{
	
}
int outtime = 0;


unsigned int __stdcall VideoEncoderNVENC::GetEncodeFrameThread(LPVOID lpParameter)
{
	//OutputDebugString(L"pico GetEncodeFrameThread");
	VideoEncoderNVENC *EncoderObj = (VideoEncoderNVENC *)lpParameter;
	int tsindex = 0;

	SetEvent(EncoderObj->mHThreadEvent);
	WaitForSingleObject(EncoderObj->mHEncoderCreateEvent, INFINITE);
	CloseHandle(EncoderObj->mHEncoderCreateEvent);
	GLOBAL_DLL_CONTEXT_LOG()->LogAlways(" WaitForSingleObject(mHEncoderCreateEvent).");
	
	//OutputDebugString(L"pico GetEncodeFrameThread loop"); 
	 
	int index = 0;
	while (EncoderObj->mEncoderRun)
	{
		int len = 0;
		uint64_t buf_index = 0;
		int ret= EncoderObj->mEncoder->GetEncodedPacketOnce(
			EncoderObj->mOutFrame[EncoderObj->out_frame_index_%OUTBUFSIZE].buf,len, buf_index,false, EncoderObj->mIndex);
		    EncoderObj->mOutFrame[EncoderObj->out_frame_index_ % OUTBUFSIZE].len = len;
			EncoderObj->mOutFrame[EncoderObj->out_frame_index_ % OUTBUFSIZE].index = buf_index;
		if (ret <= 0 )
		{			 
			string msg = "GetEncodedPacketOnce failed  %d" + to_string(ret);
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);		 
			EncoderObj->out_frame_index_++;
		}
		else
		{
			index = EncoderObj->mOutFrame[EncoderObj->out_frame_index_ % OUTBUFSIZE].index;
			gRenderPoseList[index % POSELISTSIZE].endEncodeStamp= nowInNs();
			if (gLog)
			{
				uint64_t GetFrameEnd = nowInNs();
				
				if (EncoderObj->mIndex==0)
				{
					string msg = "left Encode cost time:" + to_string((GetFrameEnd - gRenderPoseList[index % POSELISTSIZE].poseRecvTime)/1000000.f);
					GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				} 
				else
				{
					string msg = "right Encode cost time:" + to_string((GetFrameEnd - gRenderPoseList[index % POSELISTSIZE].poseRecvTime)/1000000.f);
					GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				}
				
			}
			if (gConfig.GetOutFile() == 1||gSaveVideo )
			{
				if (gConfig.GetIfHEVC() == 1)
				{
					char itype = ((EncoderObj->mOutFrame[EncoderObj->out_frame_index_ % OUTBUFSIZE].buf[4]) & 0x7E) >> 1;
					if (itype == 32)
					{
						EncoderObj->spspps_start_ = true;
					}
					if (EncoderObj->mIndex == 0)
					{
						if (EncoderObj->pleft == NULL)
						{

							EncoderObj->pleft = fopen("left.h265", "wb+");
						}
						if (EncoderObj->spspps_start_)
						{
							fwrite(EncoderObj->mOutFrame[EncoderObj->out_frame_index_ % OUTBUFSIZE].buf, sizeof(char), EncoderObj->mOutFrame[EncoderObj->out_frame_index_ % OUTBUFSIZE].len, EncoderObj->pleft);
						}

					}
					else
					{
						if (EncoderObj->pright == NULL)
						{
							EncoderObj->pright = fopen("right.h265", "wb+");
						}
						if (EncoderObj->spspps_start_)
						{
							fwrite(EncoderObj->mOutFrame[EncoderObj->out_frame_index_ % OUTBUFSIZE].buf, sizeof(char), EncoderObj->mOutFrame[EncoderObj->out_frame_index_ % OUTBUFSIZE].len, EncoderObj->pright);
						}

					}
				}
				else
				{
					char itype = ((EncoderObj->mOutFrame[EncoderObj->out_frame_index_ % OUTBUFSIZE].buf[4]) & 0x1f) ;
					if (itype == 7)
					{
						EncoderObj->spspps_start_ = true;
					}
					if (EncoderObj->mIndex == 0)
					{
						if (EncoderObj->pleft == NULL)
						{

							EncoderObj->pleft = fopen("left.h264", "wb+");
						}
						if (EncoderObj->spspps_start_)
						{
							fwrite(EncoderObj->mOutFrame[EncoderObj->out_frame_index_ % OUTBUFSIZE].buf, sizeof(char), EncoderObj->mOutFrame[EncoderObj->out_frame_index_ % OUTBUFSIZE].len, EncoderObj->pleft);
						}

					}
					else
					{
						if (EncoderObj->pright == NULL)
						{
							EncoderObj->pright = fopen("right.h264", "wb+");
						}
						if (EncoderObj->spspps_start_)
						{
							fwrite(EncoderObj->mOutFrame[EncoderObj->out_frame_index_ % OUTBUFSIZE].buf, sizeof(char), EncoderObj->mOutFrame[EncoderObj->out_frame_index_ % OUTBUFSIZE].len, EncoderObj->pright);
						}

					}
				}
			}
			 
			if (gConfig.GetIfHEVC()==0)
			{
				int ret = EncoderObj->mOutFrame[EncoderObj->out_frame_index_%OUTBUFSIZE].buf[4] & 0x1f;
				if (ret == 7)
				{
					EncoderObj->mNoIDRTime = 0;
				}
				else
				{
					EncoderObj->mNoIDRTime++;
				}
			} 
			else
			{
			
				int ret = (EncoderObj->mOutFrame[EncoderObj->out_frame_index_%OUTBUFSIZE].buf[4] & 0x7E) >> 1;
				if (ret == 32)
				{
					EncoderObj->mNoIDRTime = 0;
				}
				else
				{
					EncoderObj->mNoIDRTime++;
				}
			}
		 
			EncoderObj->out_frame_index_++;
		}
		
	}

	try
	{
		std::string msg = "delete encoder ";
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
		EncoderObj->mEncoder->DestroyEncoder();
		msg = "delete encoder ok";
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
	}
	catch (const std::exception&)
	{
		std::string msg = "delete encoder error";
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
	}
	EncoderObj->DeleteOutFrameBuffer();
	if (EncoderObj->mEncoder)
	{
		delete EncoderObj->mEncoder;
	}
	
	return 0;
}


void VideoEncoderNVENC::CreateDevice(ID3D11Device*& device)
//-----------------------------------------------------------------------------
{
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
	

	// Create the device and device context objects
	D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&device,
		&featureLevel,
		&mD3D11ContextOfEncoder);

	ID3D11Multithread *D3D11Multithread = NULL;
	HRESULT hr = mD3D11ContextOfEncoder->QueryInterface(__uuidof(ID3D11Multithread), (void **)&D3D11Multithread);
	if (SUCCEEDED(hr)) {
		D3D11Multithread->SetMultithreadProtected(TRUE);
		D3D11Multithread->Release();
	}
}
ID3D11Texture2D* VideoEncoderNVENC::CreateSharedTexture2D(ID3D11Device* device, DXGI_FORMAT format, int width, int height, int nSampleCount)
//-----------------------------------------------------------------------------
{
	ID3D11Texture2D* texture = nullptr;

	DXGI_SAMPLE_DESC sampledesc;
	sampledesc.Count = nSampleCount;
	sampledesc.Quality = 0;

	D3D11_TEXTURE2D_DESC desc;
	desc.ArraySize = 1;
	desc.CPUAccessFlags = 0;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.Format = format;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
	desc.SampleDesc = sampledesc;
	desc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&desc, 0, &texture);

	return texture;
}

HANDLE VideoEncoderNVENC::CreateSharedResource(ID3D11Texture2D* texture)
//-----------------------------------------------------------------------------
{
	HANDLE handle;
	IDXGIResource* pResource;
	texture->QueryInterface(__uuidof(IDXGIResource), (void**)&pResource);
	pResource->GetSharedHandle(&handle);
	pResource->Release();
	return handle;
}

void VideoEncoderNVENC::Initialize(const VideoEncoderConfig& config)
{	
	 
	memcpy_s(&mConfig, sizeof(VideoEncoderConfig), &config, sizeof(VideoEncoderConfig));
	GLOBAL_DLL_CONTEXT_LOG()->LogAlways("Initializing VideoEncoderNVENC.");
	mIndex = gEcoderIndex;
	if (gEcoderIndex == 0)
	{
		mHEncoderCreateEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("create Event 1"));
		mHThreadEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("send Event 1"));		
		 
	}
	else
	{
		mHEncoderCreateEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("create Event 2"));
		mHThreadEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("send Event 2"));
		 
	}
	if (gConfig.GetOutFile() == 1)
	{
		if (mIndex == 0)
		{
			pleft = fopen("left.h265", "wb+");

		}
		else
		{
			pright = fopen("right.h265", "wb+");
		}
	}

	/*ret = (HANDLE)_beginthreadex(NULL, 0, &EncodeThread, this, 0, &mEncodeThreadId);
	SetThreadPriority(ret, THREAD_PRIORITY_TIME_CRITICAL);
	WaitForSingleObject(mHThreadEvent, INFINITE);
	CloseHandle(ret);
	ret = (HANDLE)_beginthreadex(NULL, 0, &EncodeThread2, this, 0, &mEncodeThreadId2);
	SetThreadPriority(ret, THREAD_PRIORITY_TIME_CRITICAL);
	WaitForSingleObject(mHThreadEvent, INFINITE);
	CloseHandle(ret);*/
		
	if (mConfig.D3D11Device != nullptr)
	{
		mConfig.D3D11Device->GetImmediateContext(&mContextFromDeviceInConfig);
	}
	try
	{	
#ifdef  picoModel
		
		mEncoder = new NvEncoderD3D11(mConfig.D3D11Device, mConfig.width, mConfig.height, NV_ENC_BUFFER_FORMAT_ABGR,2);
		mEncoder->EncodeJudge(mEncoder->hevc_, mEncoder->h264_);
		
#else
		mEncoder = new NvEncoderD3D11(mConfig.D3D11Device, mConfig.width, mConfig.height, NV_ENC_BUFFER_FORMAT_NV12);
#endif		
		mEncodeCLIOptions = new NvEncoderInitParam();
		mInitializeParams = { NV_ENC_INITIALIZE_PARAMS_VER };
		NV_ENC_CONFIG encodeConfig = { NV_ENC_CONFIG_VER };
		mInitializeParams.encodeConfig = &encodeConfig;
		if (mEncoder->hevc_)
		{
			mEncodeCLIOptions->SetEncodeGUID(NV_ENC_CODEC_HEVC_GUID);
			std::string msg = "hevc encoder exist";
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
		} 
		else 
		{
			if (mEncoder->h264_)
			{
				std::string msg = "h264 encoder exist";
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				mEncodeCLIOptions->SetEncodeGUID(NV_ENC_CODEC_H264_GUID);
			}
			else 
			{
				std::string msg = "cuda encoder only";
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				return;
			}
		}

		if (gConfig.GetIfHEVC()==0)
		{
			mEncodeCLIOptions->SetEncodeGUID(NV_ENC_CODEC_H264_GUID);
			std::string msg = "force h264 encoder";
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
		}
	 
		mEncoder->CreateDefaultEncoderParams(&mInitializeParams, mEncodeCLIOptions->GetEncodeGUID(), mEncodeCLIOptions->GetPresetGUID());
		//mInitializeParams.encodeConfig->frameIntervalP = 0;
		mInitializeParams.frameRateNum = (uint32_t)mConfig.fps ;
		mInitializeParams.frameRateDen = 1;

		std::string msg = "gopsize:" + std::to_string(gConfig.GetGopSize());
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
		if (mEncodeCLIOptions->GetEncodeGUID() == NV_ENC_CODEC_H264_GUID)
		{
			gEncodeType = 2;
			gRtpPloadTpye = 96;
			mInitializeParams.encodeConfig->encodeCodecConfig.h264Config.enableIntraRefresh = TRUE;
			mInitializeParams.encodeConfig->encodeCodecConfig.h264Config.repeatSPSPPS = 1;
			mInitializeParams.encodeConfig->encodeCodecConfig.h264Config.level = (uint32_t)mConfig.level;
			if (gConfig.GetIfSlices() == 0)
			{
				mInitializeParams.encodeConfig->encodeCodecConfig.h264Config.sliceMode = 0;
				mInitializeParams.encodeConfig->encodeCodecConfig.h264Config.sliceModeData = 0;
			}
			else 
			{
				mInitializeParams.encodeConfig->encodeCodecConfig.h264Config.sliceMode = 3;
				mInitializeParams.encodeConfig->encodeCodecConfig.h264Config.sliceModeData = 10;
			}
			mInitializeParams.encodeConfig->rcParams.rateControlMode = (NV_ENC_PARAMS_RC_MODE)gConfig.GetRateControllModel();// NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ;// NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ;
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways("h264 end CreateDefaultEncoderParams.");
		}
		else if (mEncodeCLIOptions->GetEncodeGUID() == NV_ENC_CODEC_HEVC_GUID)
		{
			gEncodeType = 1;
			gRtpPloadTpye = 98;
			mInitializeParams.encodeConfig->encodeCodecConfig.hevcConfig.enableIntraRefresh = TRUE;
			mInitializeParams.encodeConfig->encodeCodecConfig.hevcConfig.repeatSPSPPS = 1;
			if (gConfig.GetIfSlices()==0)
			{
				mInitializeParams.encodeConfig->encodeCodecConfig.hevcConfig.sliceMode = 0;
				mInitializeParams.encodeConfig->encodeCodecConfig.hevcConfig.sliceModeData = 0;
			} 
			else
			{
				mInitializeParams.encodeConfig->encodeCodecConfig.hevcConfig.sliceMode = 3;
				mInitializeParams.encodeConfig->encodeCodecConfig.hevcConfig.sliceModeData = 10;
			}			
			mInitializeParams.encodeConfig->rcParams.rateControlMode = (NV_ENC_PARAMS_RC_MODE)gConfig.GetRateControllModel();// NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ;// NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ;
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways("hevc end CreateDefaultEncoderParams.");
		}
	
		
		mInitializeParams.encodeConfig->rcParams.zeroReorderDelay = 1;
		mInitializeParams.encodeConfig->rcParams.minQP.qpIntra = 0;
		mInitializeParams.encodeConfig->rcParams.maxQP.qpIntra = 51;
		mInitializeParams.encodeConfig->rcParams.minQP.qpInterP = 10;
		mInitializeParams.encodeConfig->rcParams.maxQP.qpInterP = 51;
		//mInitializeParams.encodeConfig->rcParams.targetQuality = 51;
		mInitializeParams.encodeConfig->rcParams.maxQP.qpInterP = 51;

		/*mInitializeParams.encodeConfig->rcParams.constQP.qpIntra = 0;
		mInitializeParams.encodeConfig->rcParams.constQP.qpInterP = 12;*/
		 
		msg = "rateControlMode:" + std::to_string(gConfig.GetRateControllModel())+":"+std::to_string(mConfig.minIQP)+":" + std::to_string(mConfig.maxPQP);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
		mInitializeParams.encodeConfig->rcParams.maxBitRate = mConfig.maxBitRate;
		mInitializeParams.encodeConfig->rcParams.averageBitRate = mConfig.avgBitRate;
		mInitializeParams.encodeConfig->gopLength =NVENC_INFINITE_GOPLENGTH;
		mInitializeParams.encodeConfig->rcParams.enableAQ = TRUE;
		mInitializeParams.enableEncodeAsync = TRUE;
		mInitializeParams.enablePTD = TRUE;
	
		msg = "frameIntervalP:" + std::to_string(mInitializeParams.encodeConfig->frameIntervalP);
		msg = "enableLTR:" + std::to_string(mInitializeParams.encodeConfig->encodeCodecConfig.hevcConfig.enableLTR);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
#ifdef  picoModel
		{
			msg = "NV_ENC_BUFFER_FORMAT_ABGR:";
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
			mEncodeCLIOptions->SetInitParams(&mInitializeParams, NV_ENC_BUFFER_FORMAT_ABGR);//NV_ENC_BUFFER_FORMAT_ABGR
		} 
#else
		{
			msg = "NV_ENC_BUFFER_FORMAT_NV12:";
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
			mEncodeCLIOptions->SetInitParams(&mInitializeParams, NV_ENC_BUFFER_FORMAT_NV12); 
		}
#endif		
		try
		{
			//mInitializeParams.encodeWidth = 400000;
			//mInitializeParams.frameRateNum = 200;
			if (mInitializeParams.encodeWidth <=mInitializeParams.maxEncodeWidth && mInitializeParams.encodeHeight <= mInitializeParams.maxEncodeHeight)
			{
				mEncoder->CreateEncoder(&mInitializeParams);
			}
			else
			{
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways("====invalid  width or height====");
			}
			
		}
		catch (NVENCException* e)
		{
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways("error error!!!!!!!");
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways(e->getErrorString());
		}
		
		HANDLE ret = (HANDLE)_beginthreadex(NULL, 0, &GetEncodeFrameThread, this, 0, NULL);
		SetThreadPriority(ret, THREAD_PRIORITY_TIME_CRITICAL);
		WaitForSingleObject(mHThreadEvent, INFINITE);
		CloseHandle(ret);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(" NVVCE CreateEncoder.");
		SetEvent(mHEncoderCreateEvent);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(" SetEvent(mHEncoderCreateEvent).");
		init_success_ = true;
	}
	catch (NVENCException* e)
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(e->getErrorString());
	}
	
	
}


void VideoEncoderNVENC::Reconfigure(const VideoEncoderConfig& config) 
{
	
	if (mConfig == config)
	{
		return;
	}

	Shutdown();
	Initialize(config);
	
}

extern RVR::RVRPoseHmdData gRenderPoseList[POSELISTSIZE];
extern uint64_t gRenderPoseIndex;
uint64_t leftlastsendtime;
uint64_t rightlastsendtime;
 
extern bool leftgDstFlag ;
extern bool rightgDstFlag  ;
extern std::string gDstIp;

unsigned int __stdcall VideoEncoderNVENC::EncodeThread2(LPVOID lpParameter)
{
	VideoEncoderNVENC * EncoderObj = (VideoEncoderNVENC*)lpParameter;
	int mSvrFrameConfigIndex = 0;
	MSG msg;
	int BufferIndex = 0;
	SetEvent(EncoderObj->mHThreadEvent);
	PeekMessage(&msg, NULL, MSG_ENCODE, MSG_ENCODE, PM_NOREMOVE);
	int encodeinedx = 0;
	while (EncoderObj->mEncoderRun)
	{
		if (GetMessage(&msg, nullptr, MSG_ENCODE, MSG_ENCODE)) //get msg from message queue
		{
			switch (msg.message)
			{
			case MSG_ENCODE:
			{

				ID3D11Texture2D *pEncodeText = EncoderObj->mEncodeText2DS[encodeinedx % 5];
				encodeinedx++;
				const NvEncInputFrame* encoderInputFrame = EncoderObj->mEncoder->GetNextInputFrame();
				ID3D11Texture2D *pTextNV12 = reinterpret_cast<ID3D11Texture2D*>(encoderInputFrame->inputPtr);
				 
				//EncoderObj->mContextFromDeviceInConfig->CopySubresourceRegion(pTextNV12, 0, 0, 0, 0, pEncodeText, 0, nullptr);
				EncoderObj->mContextFromDeviceInConfig->CopyResource(pTextNV12, pEncodeText);

				RVR::VEncFrameConfig*frameConfig = (RVR::VEncFrameConfig *)EncoderObj->mSvrFrameConfig[BufferIndex% OUTBUFSIZE];
				NV_ENC_PIC_PARAMS pPicParams = { 0 };
				 
				if (EncoderObj->mNoIDRTime >= gConfig.GetGopSize())
				{
					EncoderObj->mNoIDRTime = 0;
					pPicParams.encodePicFlags = NV_ENC_PIC_FLAG_FORCEIDR | NV_ENC_PIC_FLAG_OUTPUT_SPSPPS;

				}

				int ret = EncoderObj->mEncoder->EncodeFrame(&pPicParams);
				BufferIndex++;
			}
			case MSG_EXIT:
			{
				break;
			}
			default:
				break;
			}
		}

	}
	return 1;
}
unsigned int __stdcall VideoEncoderNVENC::EncodeThread(LPVOID lpParameter)
{
	VideoEncoderNVENC * EncoderObj = (VideoEncoderNVENC*)lpParameter;
	int mSvrFrameConfigIndex = 0;
	MSG msg;
	int BufferIndex = 0;
	SetEvent(EncoderObj->mHThreadEvent);
	PeekMessage(&msg, NULL, MSG_ENCODE, MSG_ENCODE, PM_NOREMOVE);

	while (EncoderObj->mEncoderRun)
	{
		if (GetMessage(&msg, nullptr, MSG_ENCODE, MSG_ENCODE)) //get msg from message queue
		{
			switch (msg.message)
			{
			case MSG_ENCODE: 
			{
				RVR::VEncFrameConfig*frameConfig = (RVR::VEncFrameConfig *)EncoderObj->mSvrFrameConfig[BufferIndex% OUTBUFSIZE];
				NV_ENC_PIC_PARAMS pPicParams = { 0 };
				
				if (EncoderObj->mNoIDRTime >= gConfig.GetGopSize())
				{
					EncoderObj->mNoIDRTime = 0;
					pPicParams.encodePicFlags = NV_ENC_PIC_FLAG_FORCEIDR | NV_ENC_PIC_FLAG_OUTPUT_SPSPPS;

				}
				
				int ret = EncoderObj->mEncoder->EncodeFrame(&pPicParams);
				BufferIndex++;
			}
			case MSG_EXIT: 
			{
				break;
			}
			default:
				break;
			}
		}

	}
	return 1;
}

void VideoEncoderNVENC::Transmit(ID3D11Texture2D *pTexture, VideoEncoderFrameConfig* frameConfig)
{
	if (init_success_==false|| mEncoderRun==false)
	{
		return;
	}
	 
	const NvEncInputFrame* encoderInputFrame = mEncoder->GetNextInputFrame();	 
	ID3D11Texture2D *pTextInput = reinterpret_cast<ID3D11Texture2D*>(encoderInputFrame->inputPtr);  
	mContextFromDeviceInConfig->CopySubresourceRegion(pTextInput, 0, 0, 0, 0, pTexture, 0, nullptr);
	if (gSavePicture)
	{
		if (mIndex==0)
		{
			D3DHelper::SaveTextureToFile(mContextFromDeviceInConfig, pTexture, L"left.jpg");
		} 
		else
		{
			D3DHelper::SaveTextureToFile(mContextFromDeviceInConfig, pTexture, L"right.jpg");
		}
	
	}
	 
	
	NV_ENC_PIC_PARAMS pPicParams = { 0 };
	
	if (gConfig.GetFrameControlType()==2)
	{
		pPicParams.encodePicFlags = NV_ENC_PIC_FLAG_FORCEINTRA;
	}
	if (gConfig.GetFrameControlType() == 3)
	{
		pPicParams.encodePicFlags = NV_ENC_PIC_FLAG_FORCEIDR;
	}
	if ( (mNoIDRTime >= gConfig.GetGopSize())&&(gConfig.GetFrameControlType()!=1))
	{
		mNoIDRTime = 0;
		
		if ((gConfig.GetFrameControlType() == 4) && (idr_count_ > 5 && idr_count_ % 40 != 0)) 
		{
			pPicParams.encodePicFlags = NV_ENC_PIC_FLAG_FORCEINTRA;
			
		}
		else
		{
			pPicParams.encodePicFlags = NV_ENC_PIC_FLAG_FORCEIDR | NV_ENC_PIC_FLAG_OUTPUT_SPSPPS;
			
		}
		idr_count_++;
	} 
	 
	if (frameConfig->flags & RVR::VENC_FORCE_IDR||gInsertIdr>0)
	{
		gInsertIdr--;
		mNoIDRTime = 0;
		pPicParams.encodePicFlags = NV_ENC_PIC_FLAG_FORCEIDR | NV_ENC_PIC_FLAG_OUTPUT_SPSPPS ;
		string msg = "insert idr" ;
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
		/*if (gInsertIdr <= 0)
		{
			gInsertIdr = 2;
		}
		if (gInsertIdr)
		{
			gInsertIdr = gInsertIdr-1;
		}*/
		
	}
	if (frameConfig->flags & RVR::VENC_GOP_UPDATE||gChangeGop)
	{
		int gop=gConfig.SetGopSize();
		string msg = "change gop:"+to_string(gop);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
		gChangeGop = false;
	}
	if (mNoIDRTime < 0)
	{
		pPicParams.encodePicFlags = NV_ENC_PIC_FLAG_FORCEIDR | NV_ENC_PIC_FLAG_OUTPUT_SPSPPS;
	}
	pPicParams.inputTimeStamp = gRenderPoseIndex - 1;
	int averBit;
	int maxBit;
	bool itype = false;
	if (RtpQualityHelper::GetInstance()->ChangeEncodeBitRate(mIndex,averBit, maxBit, itype))
	{
		NV_ENC_RECONFIGURE_PARAMS reConfig = {0};
		reConfig.version = NV_ENC_RECONFIGURE_PARAMS_VER;
		reConfig.reInitEncodeParams = mInitializeParams;
		reConfig.forceIDR = 1;
		reConfig.resetEncoder = 1;
		//reConfig.reInitEncodeParams.encodeConfig->gopLength = NVENC_INFINITE_GOPLENGTH;
		reConfig.reInitEncodeParams.encodeConfig->rcParams.averageBitRate = averBit;
		reConfig.reInitEncodeParams.encodeConfig->rcParams.maxBitRate = maxBit ;
		mEncoder->Reconfigure(&reConfig);
		pPicParams.encodePicFlags = NV_ENC_PIC_FLAG_FORCEIDR | NV_ENC_PIC_FLAG_OUTPUT_SPSPPS;
	}

	if ((frameConfig->flags & RVR::VENC_BITRATE_UPDATE_BY_USER) || (frameConfig->flags & RVR::VENC_BITRATE_UPDATE_BY_CONFIG))
	{
		NV_ENC_RECONFIGURE_PARAMS reConfig = { 0 };
		reConfig.version = NV_ENC_RECONFIGURE_PARAMS_VER;
		reConfig.reInitEncodeParams = mInitializeParams;
		reConfig.forceIDR = 1;
		reConfig.resetEncoder = 1;
		//reConfig.reInitEncodeParams.encodeConfig->gopLength = NVENC_INFINITE_GOPLENGTH;
		reConfig.reInitEncodeParams.encodeConfig->rcParams.averageBitRate = frameConfig->avgBitRate;
		reConfig.reInitEncodeParams.encodeConfig->rcParams.maxBitRate = frameConfig->maxBitRate;
		mEncoder->Reconfigure(&reConfig);
		pPicParams.encodePicFlags = NV_ENC_PIC_FLAG_FORCEIDR | NV_ENC_PIC_FLAG_OUTPUT_SPSPPS;
	}
	/*if (itype)
	{
		pPicParams.encodePicFlags = NV_ENC_PIC_FLAG_FORCEIDR | NV_ENC_PIC_FLAG_OUTPUT_SPSPPS;
	}*/
	int poseIndex = gRenderPoseIndex - 1;
	int ret = mEncoder->EncodeFrame(&pPicParams);
	mRenderCost[poseIndex % POSELISTSIZE] = frameConfig->timestamp;
	if (gLog)
	{
		uint64_t EncodeFrameEnd = nowInNs();
		string msg;
		if (mIndex==0)
		{
			msg = "left";
		}
		else 
		{
			msg = "right";
		}
		msg = msg +" EncodeFrame trace time:"+to_string((EncodeFrameEnd- gRenderPoseList[poseIndex % POSELISTSIZE].poseRecvTime)/1000000.f);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
	}
}

void VideoEncoderNVENC::Flush() 
{
	std::vector<std::vector<uint8_t>> vPacket;
	mEncoder->EndEncode(vPacket);
}

void VideoEncoderNVENC::Shutdown() 
{
	mEncoderRun = FALSE;
	mEncoder->DestroyEncoder();
	PostThreadMessage(mEncodeThreadId,MSG_EXIT,0,0);
	PostThreadMessage(rtp_thread_id_, MSG_EXIT, 0, 0);	
	CloseHandle(mHThreadEvent);
	
}

