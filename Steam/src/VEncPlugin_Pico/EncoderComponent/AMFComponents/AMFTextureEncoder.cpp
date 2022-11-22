#include "../RtpQualityHelper.h"
#include "AMFTextureEncoder.h"
#include "../../GlobalDLLContext.h"
#include "../../amf/public/common/Thread.h"
#include "../pxrTool/config_reader.h"
#include "../VideoEncoderFrameConfig.h"
extern int gEncodeType;
extern int gRtpPloadTpye;
extern ConfigReader gConfig;
// AMFTextureEncoder
//

AMFTextureEncoder::AMFTextureEncoder(AMFTextureReceiver receiver)
	: mReceiver(receiver)
	, mThread(nullptr)
	, mEncodeThreadShouldStop(false)
{

}

AMFTextureEncoder::~AMFTextureEncoder()
{
}

bool AMFTextureEncoder::Init(const amf::AMFContextPtr& amfContext, const VideoEncoderConfig* config)
{
	if (nullptr != mEncoder)
	{
		return false;
	}
	
	memcpy_s(&mConfig, sizeof(VideoEncoderConfig), config, sizeof(VideoEncoderConfig));
	mConfig.format = 0;
	 
	if (gConfig.GetIfHEVC()==0)
	{
		mConfig.codec =0; //h264
	} 
	else
	{
		mConfig.codec =1;
	}
	const wchar_t* pCodec = GetAMFCodecFromConfig(&mConfig);
	amf_int32 frameRateIn = config->fps;

	//TODO: Modified by AMD recommend. Fix the Qualcomm config
	amf_int32 idrPeriod = INT32_MAX;// config->CSDPeriod <= config->GOPSize ? config->CSDPeriod : config->GOPSize;
	AMF_VIDEO_ENCODER_PROFILE_ENUM profile = AMF_VIDEO_ENCODER_PROFILE_HIGH;// GetAMFCodecProfileFromConfig(config);
	//

	
	AMF_VIDEO_ENCODER_CODING_ENUM coding = GetAMFCodecCodingMethodFromConfig(&mConfig);
	 
	amf::AMF_SURFACE_FORMAT format = GetAMFCodecFormatFromConfig(config);
	//amf::AMF_SURFACE_FORMAT format= amf::AMF_SURFACE_BGRA;
	amf_int64 avgBitRateIn = gConfig.GetAverageBitRateValue(); // in bits
	amf_int64 maxBitRateIn = gConfig.GetAverageBitRateValue()+ gConfig.GetAverageBitRateValue()/10; // in bits
	GLOBAL_DLL_CONTEXT_LOG()->LogAlways("m_amfEncoder avgBitRateIn: " + std::to_string(avgBitRateIn));
	
	AMF_RESULT res = AMF_FAIL;
	// Create encoder component.
	res = g_AMFFactory.GetFactory()->CreateComponent(amfContext, pCodec, &mEncoder);
	if (res != AMF_OK)
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("CreateComponent failed.");
		return false;
	}


	if (pCodec == AMFVideoEncoderVCE_AVC)
	{
		gEncodeType = 2;
		gRtpPloadTpye = 96;
		
		 
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_LOWLATENCY_MODE, true);
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY);
		
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0);
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_LOWLATENCY_MODE, true);
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, avgBitRateIn);
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, maxBitRateIn);
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(mConfig.width, mConfig.height));
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(frameRateIn, 1));
		//mEncoder->SetProperty(AMF_VIDEO_ENCODER_CABAC_ENABLE, AMF_VIDEO_ENCODER_CABAC);//AMF_VIDEO_ENCODER_CABAC节省比特率，算法复杂度高
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, profile);
	    //mEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_MAIN);
	//	mEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, AMF_LEVEL_6_2);
		//mEncoder->SetProperty(AMF_VIDEO_ENCODER_CABAC_ENABLE, coding);
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, 0);
		AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM rate_control = GetAVCRateControlMode(gConfig.GetRateControllModel());
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, rate_control);
		/*if (gConfig.GetTcpValue())
		{
			mEncoder->SetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, 0);
		}*/
		//mEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, rcMode);

		
		//mEncoder->SetProperty(AMF_VIDEO_ENCODER_MIN_QP, minQP);
		//mEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_QP, maxQP);
       if (gConfig.GetIfSlices()==1)
       {
		   mEncoder->SetProperty(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, 6);
       }
	
		
	}
	else
	{	 
		gEncodeType = 1;
		gRtpPloadTpye = 98;
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_USAGE, AMF_VIDEO_ENCODER_HEVC_USAGE_LOW_LATENCY);
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_FRAMESIZE, ::AMFConstructSize(mConfig.width, mConfig.height));
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_FRAMERATE, ::AMFConstructRate(frameRateIn, 1));
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, AMF_LEVEL_6);
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET, AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_SPEED);
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_SKIP_FRAME_ENABLE, false);	
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_TARGET_BITRATE, avgBitRateIn);		 
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_PEAK_BITRATE, maxBitRateIn);
		AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_ENUM rate_control = GetHEVCRateControlMode(gConfig.GetRateControllModel());
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD, rate_control);
	///*mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD, AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_QUALITY_VBR);	
	//	mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_MAX_QP_P, 50);
	//	mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_MAX_QP_I, 50);
	//	mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_MIN_QP_P, 0);
	//	mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_MIN_QP_I, 0); 
	//	mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_QP_I, 8);
	//	mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_QP_P, 8);*/
		
	//	else if ((int64_t)20000000<avgBitRateIn && avgBitRateIn <= (int64_t)40000000)
	//	{
	//		mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_MIN_QP_P, 24);
	//		mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_MIN_QP_I, 22);
	//		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(" qp 2");
	//		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("  avgBitRateIn: " + std::to_string(avgBitRateIn));
	//	}
	//	else  if ((int64_t)40000000 < avgBitRateIn)
	//	{
	//		mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_MIN_QP_P, 23);
	//		mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_MIN_QP_I, 23);
	//		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(" qp 3");
	//		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("  avgBitRateIn: " + std::to_string(avgBitRateIn));
	//	}*/
		mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_GOP_SIZE, 0);
		/*if (gConfig.GetTcpValue())
		{
			mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_GOP_SIZE, 0);
		}*/
		//mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_ENABLE_VBAQ, true);
 //
		if (gConfig.GetIfSlices() > 0)
		{
			 mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_SLICES_PER_FRAME, 6);
		} 
		
	}

	res = mEncoder->Init(format, mConfig.width, mConfig.height);
 
	if (res != AMF_OK)
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("Init failed. res: " + std::to_string(res));
		return false;
	}

	GLOBAL_DLL_CONTEXT_LOG()->LogAlways("Initialized AMFTextureEncoder.");
	return true;
}

void AMFTextureEncoder::Start()
{
	if (nullptr == mThread)
	{
		mThread = new std::thread(&AMFTextureEncoder::Run, this);
	}
}

void AMFTextureEncoder::Shutdown()
{
	mEncodeThreadShouldStop = true;
	
	if (nullptr != mEncoder)
	{
		mEncoder->Drain();
		mEncoder->Release();
	}

	if (nullptr != mThread)
	{
		mThread->join();
		delete mThread;
		mThread = nullptr;
	}
}

void AMFTextureEncoder::Submit(amf::AMFData* data, VideoEncoderFrameConfig* frameConfig)
{
	int averBit;
	int maxBit;
	bool itype = false;
	SetBitRate(frameConfig);
	/*if (itype)
	{
		if (gConfig.GetIfHEVC() == 0)
		{
			data->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_PICTURE_TYPE_ENUM::AMF_VIDEO_ENCODER_PICTURE_TYPE_IDR);
		}
		else
		{
			data->SetProperty(AMF_VIDEO_ENCODER_HEVC_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_ENUM::AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_I);
		}
	}*/
	if (mEncoder == nullptr)
	{
		return;
	}
	
	auto res = mEncoder->SubmitInput(data);
	if (res != AMF_OK)
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("m_amfEncoder->SubmitInput returns error: " + std::to_string(res));
	}
}
void AMFTextureEncoder::SetBitRate(VideoEncoderFrameConfig* frameConfig) 
{
	if ((frameConfig->flags & RVR::VENC_BITRATE_UPDATE_BY_USER) || (frameConfig->flags & RVR::VENC_BITRATE_UPDATE_BY_CONFIG)
		|| (frameConfig->flags & RVR::VENC_BITRATE_UPDATE_BY_NET))
	{
		if (gConfig.GetIfHEVC() == 0)
		{
			mEncoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, frameConfig->avgBitRate);
			mEncoder->SetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, frameConfig->maxBitRate);
		}
		else
		{
			mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_TARGET_BITRATE, frameConfig->avgBitRate);
			mEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_PEAK_BITRATE, frameConfig->maxBitRate);
		}

		string msg = "ChangeEncodeBitRateByUser or config flag"+to_string(frameConfig->flags)+"eye index"+to_string(mIndex) + "average" + to_string(frameConfig->avgBitRate) + "max" + to_string(frameConfig->maxBitRate);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
	}
}
void AMFTextureEncoder::Flush()
{
	if (mEncoder != nullptr)
	{
		mEncoder->Flush();
	}
}

amf::AMF_SURFACE_FORMAT AMFTextureEncoder::GetFormat()
{
	return GetAMFCodecFormatFromConfig(&mConfig);
}

const wchar_t* AMFTextureEncoder::GetAMFCodecFromConfig(const VideoEncoderConfig* config)
{
	const wchar_t* pCodec = nullptr;
	switch (config->codec) {
	case 0:
		pCodec = AMFVideoEncoderVCE_AVC;
		break;
	case 1:
		pCodec = AMFVideoEncoder_HEVC;
		break;
	}

	return pCodec;
}

AMF_VIDEO_ENCODER_PROFILE_ENUM AMFTextureEncoder::GetAMFCodecProfileFromConfig(const VideoEncoderConfig* config)
{
	AMF_VIDEO_ENCODER_PROFILE_ENUM amfProfile;
	switch (config->profile)
	{
	case 0:
		amfProfile = AMF_VIDEO_ENCODER_PROFILE_BASELINE;
		break;
	case 1:
		amfProfile = AMF_VIDEO_ENCODER_PROFILE_MAIN;
		break;
	case 2:
		amfProfile = AMF_VIDEO_ENCODER_PROFILE_HIGH;
		break;
	case 3:
		amfProfile = AMF_VIDEO_ENCODER_PROFILE_CONSTRAINED_BASELINE;
		break;
	default:
		amfProfile = AMF_VIDEO_ENCODER_PROFILE_BASELINE;
	}

	return amfProfile;
}

AMF_VIDEO_ENCODER_CODING_ENUM AMFTextureEncoder::GetAMFCodecCodingMethodFromConfig(const VideoEncoderConfig* config)
{
	AMF_VIDEO_ENCODER_CODING_ENUM coding = AMF_VIDEO_ENCODER_UNDEFINED;
	switch (config->entropyCodingMode) {
	case 0:
		coding = AMF_VIDEO_ENCODER_CALV;
		break;
	case 1:
		coding = AMF_VIDEO_ENCODER_CABAC;
		break;
	}

	return coding;
}

amf::AMF_SURFACE_FORMAT AMFTextureEncoder::GetAMFCodecFormatFromConfig(const VideoEncoderConfig* config)
{
	amf::AMF_SURFACE_FORMAT format = amf::AMF_SURFACE_RGBA;
	switch (config->format) {
	case 0:
		format = amf::AMF_SURFACE_RGBA;
		break;
	case 1:
		format = amf::AMF_SURFACE_NV12;
		break;
	case 2:
		format = amf::AMF_SURFACE_P010;
		break;
	case 3:
		format = amf::AMF_SURFACE_UNKNOWN;
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("Format P016 acquired but AMF doesn't support.");
		break;
	}
	format = amf::AMF_SURFACE_RGBA;
	return format;
}
//
//AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM AMFTextureEncoder::GetAMFCodecRateControlModeFromConfig(const VideoEncoderConfig* config)
//{
//	AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM rcMode = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_UNKNOWN;
//	switch (config->rcMode) {
//	case 0:
//		rcMode = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR;
//		break;
//	case 1:
//		rcMode = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
//		break;
//	default:
//		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("RC mode acquired but AMF doesn't support.");
//		rcMode = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_UNKNOWN;
//	}
//
//	return rcMode;
//}
AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM AMFTextureEncoder::GetAVCRateControlMode(int rvr_config) 
{
	AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM ret_val = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
	switch (rvr_config)
	{
	case 0:
		ret_val = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP;
		break;
	case 1:
		ret_val = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
		break;
	case 2:
		ret_val = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR;
		break;
	case 4:
		ret_val = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR;
		break;
	case 8:
	case 16:
		ret_val = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR;
		break;
	case 32:
		ret_val = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
		break;
	default:
		ret_val = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
		break;
	}
	std::string msg = "amd get avc ratecontrol tyep=" + std::to_string((int)(ret_val));
	GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
	return ret_val;
}
AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_ENUM  AMFTextureEncoder::GetHEVCRateControlMode(int rvr_config) 
{
	AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_ENUM ret_val = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
	switch (rvr_config)
	{
	case 0:
		ret_val = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CONSTANT_QP;
		break;
	case 1:
		ret_val = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
		break;
	case 2:
		ret_val = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CBR;
		break;
	case 4:
		ret_val = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR;
		break;
	case 8:
	case 16:
		ret_val = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CBR;
		break;
	case 32:
		ret_val = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
		break;
	default:
		ret_val = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
		break;
	}
	std::string msg = "amd get hevc ratecontrol tyep=" + std::to_string((int)(ret_val));
	GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
	return ret_val;
}
void AMFTextureEncoder::Run()
{
	GLOBAL_DLL_CONTEXT_LOG()->LogTrace("Start AMFTextureEncoder thread.");
	amf::AMFDataPtr data;
	while (!mEncodeThreadShouldStop)
	{
		auto res = mEncoder->QueryOutput(&data);
		if (res == AMF_REPEAT)
		{
			amf_sleep(1);
			continue;
		}

		if (res == AMF_OK)
		{
			if (data != NULL)
			{
				mReceiver(data);
				GLOBAL_DLL_CONTEXT_LOG()->LogTrace(std::to_string((uint64_t)this) + " VideoEncoderVCE::Receiver end.");

			}
			else
			{
				amf_sleep(1);
			}
		}
		else
		{
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways(" AMFTextureEncoder exit, QueryOutput returns error: " + std::to_string(res));
			break;
		}
	}
	GLOBAL_DLL_CONTEXT_LOG()->LogTrace("Quit AMFTextureEncoder thread.");
}
