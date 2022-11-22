#include "../RtpPacket.h"
#include "VideoEncoderVCE.h"
#include "../../GlobalDLLContext.h"
#include "../RVRPlugin/IEncOutputSink.h"
#include "../VideoEncoderFrameConfig.h"
#include "../pxrTool/config_reader.h"
#include "../D3DHelper.h"
#include "../pxrTool/TimeTool.h"
#include "../../RtpQualityHelper.h"
#include "../RateDetecter.h"
const wchar_t* VideoEncoderVCE::FRAME_INDEX = L"FrameIndex";
const wchar_t* VideoEncoderVCE::START_TIME_PROPERTY = L"StartTimeProperty";
const wchar_t* VideoEncoderVCE::VIDEO_FRAME_CONFIG_POINTER_PROPERTY = L"VideoFrameConfigPointerProperty";
extern PushEncodedFrameFun gPushEncodedFrameFun;

extern uint64_t gRenderPoseIndex;
extern ConfigReader gConfig;
extern int gInsertIdr ;
extern bool gChangeGop ;
extern bool gChagneBitRate  ;
extern bool gSavePicture;
extern bool gSaveVideo;
extern SendIndex gSendIndex;
extern RVR::RVRPoseHmdData gRenderPoseList[POSELISTSIZE];
extern uint64_t gRenderPoseIndex;
extern bool gLog;
extern int gRtpPloadTpye;
//
// VideoEncoderVCE
//
int gEncodeType = 1;//1 265 2 264
int gRtpPloadTpye = 98;
VideoEncoderVCE::VideoEncoderVCE()
{
}

VideoEncoderVCE::~VideoEncoderVCE()
{}



void VideoEncoderVCE::Initialize(const VideoEncoderConfig& config)
{
	GLOBAL_DLL_CONTEXT_LOG()->LogTrace("Initializing VideoEncoderVCE.");
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
	mNoIDRTime = -1;
	memcpy_s(&mConfig, sizeof(VideoEncoderConfig), &config, sizeof(VideoEncoderConfig));
	 

	if (mConfig.D3D11Device != nullptr)
	{
		mConfig.D3D11Device->GetImmediateContext(&contextFromDeviceInConfig);
	}

	{
		AMF_RESULT res = AMF_FAIL;
		res = g_AMFFactory.Init();
		if (res != AMF_OK)
		{
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways("AMFFactory.Init failed.");
		}

		::amf_increase_timer_precision();

		res = g_AMFFactory.GetFactory()->CreateContext(&mContext);
		if (res != AMF_OK)
		{
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways("AMFFactory.CreateContext failed.");
		}

		res = mContext->InitDX11(mConfig.D3D11Device);
		if (res != AMF_OK)
		{
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways("InitDX11 failed.");
			init_success_ = false;
		}

		int roiLevel = GLOBAL_DLL_CONTEXT()->GetGlobalDLLConfig().GetFixedFoveatedEncode();
		 
	}

	/*{
		mSender = std::make_shared<AsyncDataProcessor>();
		mSender->SetProcessFunction(std::bind(&VideoEncoderVCE::DeferSend, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		mSender->Start();
	}*/
	
	mEncoder = std::make_shared<AMFTextureEncoder>(std::bind(&VideoEncoderVCE::Receive, this, std::placeholders::_1));
	if (gConfig.GetIfHEVC() == 1) //h265??
	{
		mConfig.codec = 1;
	}
	else
	{
		mConfig.codec = 0;
	}
	if (mEncoder->Init(mContext, &mConfig))
	{
		mEncoder->Start();
		mEncoder->mIndex = mIndex;
		init_success_ = true;

	}
	else
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("Failure initialized VideoEncoderVCE.");
		init_success_ = false;
	}
	
}

void VideoEncoderVCE::Reconfigure(const VideoEncoderConfig& config)
{
	if (mConfig == config)
	{
		return;
	}

	Shutdown();
	Initialize(config);
}

void VideoEncoderVCE::Shutdown()
{
	GLOBAL_DLL_CONTEXT_LOG()->LogTrace("Shutting down VideoEncoderVCE.");

	mEncoder->Shutdown();

	//mSender->Stop();
	//mSender = nullptr;

	if (nullptr != contextFromDeviceInConfig)
	{
		contextFromDeviceInConfig->Release();
		contextFromDeviceInConfig = nullptr;
	}

	amf_restore_timer_precision();

	GLOBAL_DLL_CONTEXT_LOG()->LogTrace("Successfully shutdown VideoEncoderVCE.");
}

void VideoEncoderVCE::Flush()
{
	if (mEncoder != nullptr)
	{
		mEncoder->Flush();
	}
}
 
void VideoEncoderVCE::Transmit(ID3D11Texture2D* pTexture, VideoEncoderFrameConfig* frameConfig)
{
	if (init_success_==false)
	{
		return;
	}
	
	GLOBAL_DLL_CONTEXT_LOG()->LogTrace("VideoEncoderVCE::Transmit.");
	//amf::AMFSurfacePtr surface = surface_buf_[surface_index_];
	//surface_index_++;
	//surface_index_ = surface_index_ % surfacenum;
	amf::AMFSurfacePtr surface = nullptr;
	//if (surface ==nullptr)
	{
		auto amfRes = AMF_FAIL;
		amfRes = mContext->AllocSurface(amf::AMF_MEMORY_DX11, mEncoder->GetFormat(), mConfig.width, mConfig.height, &surface);
		if (amfRes != AMF_OK)
		{
			GLOBAL_DLL_CONTEXT_LOG()->LogError("AMF AllocSurface Failed.");
			return;
		}
	}
	
	ID3D11Texture2D* textureDX11 = (ID3D11Texture2D*)surface->GetPlaneAt(0)->GetNative(); // no reference counting - do not Release()
	
	if (nullptr != contextFromDeviceInConfig)
	{	
		contextFromDeviceInConfig->CopySubresourceRegion(textureDX11, 0, 0, 0, 0, pTexture, 0, nullptr);
       // D3DHelper::SaveTextureToFile(contextFromDeviceInConfig, textureDX11, L"D://1D.jpg");
	}
	if (gSavePicture)
	{
		if (mIndex == 0)
		{
			D3DHelper::SaveTextureToFile(contextFromDeviceInConfig, pTexture, L"left.jpg");
		}
		else
		{
			D3DHelper::SaveTextureToFile(contextFromDeviceInConfig, pTexture, L"right.jpg");
		}

	}
	amf_pts start_time = amf_high_precision_clock();
	surface->SetProperty(START_TIME_PROPERTY, start_time);

	amf_uint64 ptr = (amf_uint64)frameConfig->originPointer;
	surface->SetProperty(VIDEO_FRAME_CONFIG_POINTER_PROPERTY, ptr);
	
	amf_uint64 frame_index = (amf_uint64)(gRenderPoseIndex - 1);
	surface->SetProperty(FRAME_INDEX, frame_index);
	if (gConfig.GetIfHEVC() == 0)
	{
		surface->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_PICTURE_TYPE_ENUM::AMF_VIDEO_ENCODER_PICTURE_TYPE_P);
	}
	else
	{
		surface->SetProperty(AMF_VIDEO_ENCODER_HEVC_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_ENUM::AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_P);
	}
	
	if (gConfig.GetFrameControlType() == 2)
	{
		if (gConfig.GetIfHEVC() == 0)
		{
			surface->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_PICTURE_TYPE_ENUM::AMF_VIDEO_ENCODER_PICTURE_TYPE_I);
		}
		else 
		{
			surface->SetProperty(AMF_VIDEO_ENCODER_HEVC_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_ENUM::AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_I);
		}
	}
	if (gConfig.GetFrameControlType() == 3)
	{
		if (gConfig.GetIfHEVC() == 0)
		{
			surface->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_PICTURE_TYPE_ENUM::AMF_VIDEO_ENCODER_PICTURE_TYPE_IDR);
		}
		else
		{
			surface->SetProperty(AMF_VIDEO_ENCODER_HEVC_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_ENUM::AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_IDR);
		}
	}

	if ((mNoIDRTime >= gConfig.GetGopSize()) && (gConfig.GetFrameControlType() != 1))
	{
		
		if (gConfig.GetIfHEVC()==0)
		{	
			if ((gConfig.GetFrameControlType() == 4) && (idr_count_ > 5 && idr_count_ % 40 != 0))
			{
				surface->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_PICTURE_TYPE_ENUM::AMF_VIDEO_ENCODER_PICTURE_TYPE_I);
				
			}
			else
			{
				surface->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_PICTURE_TYPE_ENUM::AMF_VIDEO_ENCODER_PICTURE_TYPE_IDR);
				
			}
			
			surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_SPS, true);
			surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_PPS, true);			
			
		}
		else
		{	
			if ((gConfig.GetFrameControlType() == 4)&&(idr_count_>5&&idr_count_%40!=0))
			{
				surface->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_ENUM::AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_I);
				
			}
			else
			{
				surface->SetProperty(AMF_VIDEO_ENCODER_HEVC_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_ENUM::AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_IDR);
				surface->SetProperty(AMF_VIDEO_ENCODER_HEVC_INSERT_HEADER, true);
				
			}
						
		}
		idr_count_++;
	}
	
	 
	if (frameConfig->flags & RVR::VENC_FORCE_IDR||gInsertIdr>0)
	{
		gInsertIdr--;
		if (gConfig.GetIfHEVC() == 0)
		{
			surface->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_PICTURE_TYPE_IDR);
			surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_SPS, true);
			surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_PPS, true);
		}
		else
		{
			surface->SetProperty(AMF_VIDEO_ENCODER_HEVC_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_IDR);
			surface->SetProperty(AMF_VIDEO_ENCODER_HEVC_INSERT_HEADER, true);
		}
		string msg = "insert idr";
		GLOBAL_DLL_CONTEXT_LOG()->LogWarning(msg);
	
	}
	if (frameConfig->flags & RVR::VENC_GOP_UPDATE||gChangeGop)
	{
		gChangeGop = false;
		int gop = gConfig.SetGopSize();
		string msg = "change gop:" + to_string(gop);
		GLOBAL_DLL_CONTEXT_LOG()->LogWarning(msg);
	}
	if (gConfig.GetIfHEVC()==0)
	{
		surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_AUD, false);
	} 
	else
	{
		surface->SetProperty(AMF_VIDEO_ENCODER_HEVC_INSERT_AUD, false);
	}
	if (mNoIDRTime < 0)
	{
		if (gConfig.GetIfHEVC() == 0)
		{
			surface->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_PICTURE_TYPE_ENUM::AMF_VIDEO_ENCODER_PICTURE_TYPE_IDR);
		}
		else
		{
			surface->SetProperty(AMF_VIDEO_ENCODER_HEVC_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_ENUM::AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_IDR);
		}
	}
	/*if (mRoiSurface != nullptr)
	{
		ApplyFrameROI(surface, mRoiSurface);
	}*/
	
	mEncoder->Submit(surface, frameConfig);
	int poseIndex = gRenderPoseIndex - 1;
	mRenderCost[poseIndex % POSELISTSIZE] = frameConfig->timestamp;
	if (gLog)
	{
		int64_t EncodeFrameEnd = nowInNs();
		string msg;
		if (mIndex == 0)
		{
			msg = "left";
		}
		else
		{
			msg = "right";
		}
		msg = msg + " EncodeFrame trace time:" + to_string((EncodeFrameEnd - gRenderPoseList[poseIndex % POSELISTSIZE].poseRecvTime) / 1000000.f);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
	}
}
void VideoEncoderVCE::SetBitRate(VideoEncoderFrameConfig* frameConfig)
{

}
void VideoEncoderVCE::Receive(amf::AMFData* data)
{
	amf_uint64 frameConfPtr = 0;
	data->GetProperty(VIDEO_FRAME_CONFIG_POINTER_PROPERTY, &frameConfPtr);

	amf::AMFBufferPtr buffer(data); // query for buffer interface

	amf_pts current_time = amf_high_precision_clock();
	amf_pts start_time = 0;
	data->GetProperty(START_TIME_PROPERTY, &start_time);

	amf_uint64 index = 0;
	data->GetProperty(FRAME_INDEX, &index);
	// std::string log_msg = "VCE encode latency: " + std::to_string(double(current_time - start_time) / (double)MILLISEC_TIME) + "ms";
	// GLOBAL_DLL_CONTEXT_LOG()->LogInfo(log_msg);
  
	auto* p = static_cast<unsigned char*>(buffer->GetNative());
	auto length = static_cast<int>(buffer->GetSize());
	
    if(memcmp(p, "\x00\x00\x00\x01\x46", 5)== 0&&length==7) 
    {
		string msg = "adu len" + to_string(length);
		GLOBAL_DLL_CONTEXT_LOG()->LogInfo(msg);
		 
		return;
	}
	if (memcmp(p, "\x00\x00\x00\x01\x09\x10", 6) == 0 && length == 6)
	{
		string msg = "adu len" + to_string(length);
		GLOBAL_DLL_CONTEXT_LOG()->LogInfo(msg);
		return;
	}
	SkipAUD(&p, &length);
	gRenderPoseList[index % POSELISTSIZE].endEncodeStamp=nowInNs();
	/*string msg;
	if (mIndex == 0)
	{
		msg = "left";
	}
	else
	{
		msg = "right";
	}
	msg = msg + " new EncodeFrame trace time:" + to_string((gRenderPoseList[index % POSELISTSIZE].endEncodeStamp - gRenderPoseList[index % POSELISTSIZE].beginEncodeStamp) / 1000000.f);
	GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);*/
	if (gLog)
	{
		uint64_t GetFrameEnd = nowInNs();
		if (mIndex==0)
		{
			string msg = "left Encode cost time:" + to_string((GetFrameEnd - gRenderPoseList[index % POSELISTSIZE].poseRecvTime) / 1000000.f);
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
		}
		else
		{
			string msg = "right Encode cost time:" + to_string((GetFrameEnd - gRenderPoseList[index % POSELISTSIZE].poseRecvTime) / 1000000.f);
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
		}

	}
	if (gConfig.GetRtcOrBulkMode_()!=0)
	{
		EncodeOutWithInfo encode_info;
		encode_info.autoRateFlag = gConfig.GetAutoRateValue();
		int bit_rate = 0;
		int max_rate = 0;
		//RtpQualityHelper::GetInstance()->GetCurrentRate(bit_rate, max_rate);
		int auto_rate = gConfig.GetAutoRateValue();
		if (auto_rate == 1)
		{
			bit_rate = RateDetecter::GetInstance()->GetCurrentAverageRate();
		}
		else
		{
			bit_rate = gConfig.GetAverageBitRateValue();
		}
		encode_info.bitRate = bit_rate;
		encode_info.encodEnd = nowInNs() ;
		if (gConfig.BigPicture() == 1)
		{
			encode_info.index = index;
		}
		else
		{
			encode_info.index = out_inedx_;
		}
		encode_info.PloadTpye = gRtpPloadTpye;
		gPushEncodedFrameFun((char*)p, length, mIndex,encode_info);
	}
	memmove(mOutFrame[out_frame_index_ % OUTBUFSIZE].buf, p, length);
	mOutFrame[out_frame_index_ % OUTBUFSIZE].len = length;
	mOutFrame[out_frame_index_ % OUTBUFSIZE].index = index;
	out_inedx_ ++;
	if (gConfig.GetOutFile() == 1 || gSaveVideo)
	{
		if (gConfig.GetIfHEVC() == 1)
		{
			char itype = ((mOutFrame[out_frame_index_ % OUTBUFSIZE].buf[4]) & 0x7E) >> 1;
			if (itype == 32)
			{
				spspps_start_ = true;
			}
			if (mIndex == 0)
			{
				if (pleft == NULL)
				{

					pleft = fopen("left.h265", "wb+");
				}
				if (spspps_start_)
				{
					fwrite(mOutFrame[out_frame_index_ % OUTBUFSIZE].buf, sizeof(char), mOutFrame[out_frame_index_ % OUTBUFSIZE].len, pleft);
				}

			}
			else
			{
				if (pright == NULL)
				{
					pright = fopen("right.h265", "wb+");
				}
				if (spspps_start_)
				{
					fwrite(mOutFrame[out_frame_index_ % OUTBUFSIZE].buf, sizeof(char), mOutFrame[out_frame_index_ % OUTBUFSIZE].len, pright);
				}

			}
		}
		else 
		{
			char itype = ((mOutFrame[out_frame_index_ % OUTBUFSIZE].buf[4]) & 0x1f)  ;
			if (itype == 7)
			{
				spspps_start_ = true;
			}
			if (mIndex == 0)
			{
				if (pleft == NULL)
				{

					pleft = fopen("left.h264", "wb+");
				}
				if (spspps_start_)
				{
					fwrite(mOutFrame[out_frame_index_ % OUTBUFSIZE].buf, sizeof(char), mOutFrame[out_frame_index_ % OUTBUFSIZE].len, pleft);
				}

			}
			else
			{
				if (pright == NULL)
				{
					pright = fopen("right.h264", "wb+");
				}
				if (spspps_start_)
				{
					fwrite(mOutFrame[out_frame_index_ % OUTBUFSIZE].buf, sizeof(char), mOutFrame[out_frame_index_ % OUTBUFSIZE].len, pright);
				}

			}
		}
		
	}

	
	if (gConfig.GetIfHEVC() == 0)
	{
		int ret = mOutFrame[out_frame_index_ % OUTBUFSIZE].buf[4] & 0x1f;
		if (ret == 7)
		{
			mNoIDRTime = 0;
		}
		else
		{
			mNoIDRTime++;
		}
	}
	else
	{

		int ret = (mOutFrame[out_frame_index_ % OUTBUFSIZE].buf[4] & 0x7E) >> 1;
		if (ret == 32)
		{
			mNoIDRTime = 0;
		}
		else
		{
			mNoIDRTime++;
		}
	}
	{
		std::unique_lock<std::mutex> lock(frame_index_mutex_);
		last_produce_time_ns_ = nowInNs();
		out_frame_index_++;
		frame_index_cv_.notify_one();
	}
	//PostThreadMessage(rtp_thread_id_, MSG_PACKET_AND_SEND, (WPARAM) & (mOutFrame[out_frame_index_ % OUTBUFSIZE].index), (LPARAM) & (mIndex));


}

void VideoEncoderVCE::DeferSend(const void* data, size_t len, void* userData)
{

	if (mConfig.reserved != nullptr)
	{
		RVR::IEncOutputSink* encOutputSink = static_cast<RVR::IEncOutputSink*>(mConfig.reserved);
		RVR::VEncFrameConfig* frameConfig = static_cast<RVR::VEncFrameConfig*>((void*)userData);

		encOutputSink->ProcessSample(data, len, frameConfig, nullptr, mConfig.sliceSize);
	}
}

void VideoEncoderVCE::ApplyFrameType(const amf::AMFSurfacePtr& surface, VideoEncoderFrameConfig* frameConfig) {
	switch (mConfig.codec) {
	case 0:
	{
		// Disable AUD (NAL Type 9) to produce the same stream format as VideoEncoderNVENC.
		surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_AUD, false);

		if (frameConfig->flags & RVR::VENC_GOP_UPDATE
			) {
			//TODO:Using Bitrate for first connected message, as driver.dll does.but not workable.
			set_idr_resend_count(5);  //resend 5 times idr at hmd startup
		}

			
		if (frameConfig->flags & RVR::VENC_FORCE_INTRA
			) {
			GLOBAL_DLL_CONTEXT_LOG()->LogInfo("Inserting I frame for H.264.");
			surface->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_PICTURE_TYPE_I);
		}
		if (frameConfig->flags & RVR::VENC_FORCE_IDR
			) {
			GLOBAL_DLL_CONTEXT_LOG()->LogInfo("Inserting IDR frame for H.264.");
			surface->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_PICTURE_TYPE_IDR);
		}
		if (frameConfig->flags & RVR::VENC_PREPEND_CSD)
		{
			surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_SPS, true);
			surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_PPS, true);
			GLOBAL_DLL_CONTEXT_LOG()->LogInfo("Inserting CSD of stream for H.264.");
		}
		if (frameConfig->flags & RVR::VENC_INSERT_SEI)
		{
			GLOBAL_DLL_CONTEXT_LOG()->LogInfo("Inserting SEI of stream for H.264.");
		}
		if (frameConfig->flags & RVR::VENC_END_OF_STREAM)
		{
			GLOBAL_DLL_CONTEXT_LOG()->LogInfo("Inserting end of stream for H.264.");
			surface->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_END_OF_STREAM);
		}
	}
	break;
	}
}

void VideoEncoderVCE::ApplyFrameROI(const amf::AMFSurfacePtr& surface, const amf::AMFSurfacePtr& roiSurface)
{	
	surface->SetProperty(AMF_VIDEO_ENCODER_ROI_DATA, roiSurface);
}

void VideoEncoderVCE::SkipAUD(unsigned char** buffer, int* length) {
	// H.265 encoder always produces AUD NAL even if AMF_VIDEO_ENCODER_HEVC_INSERT_AUD is set. But it is not needed.
	static const int AUD_NAL_SIZE = 7;



	if (mConfig.codec != 1) {
		return;
	}

	if (*length < AUD_NAL_SIZE + 4) {
		return;
	}

	// Check if start with AUD NAL.
	if (memcmp(*buffer, "\x00\x00\x00\x01\x46", 5) != 0) {
		return;
	}
	// Check if AUD NAL size is AUD_NAL_SIZE bytes.
	if (memcmp(*buffer + AUD_NAL_SIZE, "\x00\x00\x00\x01", 4) != 0) {
		return;
	}
	*buffer += AUD_NAL_SIZE;
	*length -= AUD_NAL_SIZE;
}

amf::AMFSurfacePtr VideoEncoderVCE::CreateROISurface(int surfaceWidth, int surfaceHeight, int roiLevel)
{
	AMF_RESULT res = AMF_OK;
	amf::AMFSurfacePtr roiSurface;
	amf::AMF_SURFACE_FORMAT roiFormat = amf::AMF_SURFACE_GRAY32;
	amf_int32 roiBlocksX;
	amf_int32 roiBlocksY;

	if (gConfig.GetIfHEVC() == 0) 
	{
		roiBlocksX = (surfaceWidth + 15) >> 4;
		roiBlocksY = (surfaceHeight + 15) >> 4;

	}
	else
	{
		roiBlocksX = (surfaceWidth + 15) >> 8;
		roiBlocksY = (surfaceHeight + 15) >> 8;
	}

	//Allocate ROI map surface
	res = mContext->AllocSurface(amf::AMF_MEMORY_HOST, roiFormat, roiBlocksX, roiBlocksY, &roiSurface);
	if (res != AMF_OK)
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogError("AMFContext::AllocSurface(amf::AMF_MEMORY_HOST) for ROI map failed!");
	}

	amf_uint32* buf = (amf_uint32*)roiSurface->GetPlaneAt(0)->GetNative();
	amf_int32 pitch = roiSurface->GetPlaneAt(0)->GetHPitch();
	memset((void*)buf, 0, pitch * roiBlocksY);

	for (int y = roiBlocksY / roiLevel; y < roiBlocksY * (roiLevel - 1) / roiLevel; y++)
	{
		for (int x = roiBlocksX / roiLevel; x < roiBlocksX * (roiLevel - 1) / roiLevel; x++)
		{
			buf[x + y * pitch / 4] = 10;
		}
	}

	return roiSurface;
}
