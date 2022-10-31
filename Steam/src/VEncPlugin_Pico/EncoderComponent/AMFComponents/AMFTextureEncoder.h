#pragma once

#include "../../amf/public/common/AMFFactory.h"
#include "../../amf/public/include/components/VideoEncoderVCE.h"
#include "../../amf/public/include/components/VideoEncoderHEVC.h"
#include "../../amf/public/common/AMFSTL.h"
#include "AMFTypeDefines.h"
#include "../VideoEncoderConfig.h"
#include "../VideoEncoderFrameConfig.h"

#include <thread>


class AMFTextureEncoder {
public:
	AMFTextureEncoder(AMFTextureReceiver receiver);
	~AMFTextureEncoder();

	bool Init(const amf::AMFContextPtr& amfContext, const VideoEncoderConfig* config);
	void Start();
	void Shutdown();
	void Submit(amf::AMFData* data, VideoEncoderFrameConfig* frameConfig);
	void Flush();
	amf::AMF_SURFACE_FORMAT GetFormat();
	int mIndex = 0;
private:
	amf::AMFComponentPtr mEncoder;
	std::thread* mThread;
	bool mEncodeThreadShouldStop;
	AMFTextureReceiver mReceiver;
	VideoEncoderConfig mConfig;

	const wchar_t* GetAMFCodecFromConfig(const VideoEncoderConfig* config);
	AMF_VIDEO_ENCODER_PROFILE_ENUM GetAMFCodecProfileFromConfig(const VideoEncoderConfig* config);
	AMF_VIDEO_ENCODER_CODING_ENUM GetAMFCodecCodingMethodFromConfig(const VideoEncoderConfig* config);
	amf::AMF_SURFACE_FORMAT GetAMFCodecFormatFromConfig(const VideoEncoderConfig* config);
	AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM GetAMFCodecRateControlModeFromConfig(const VideoEncoderConfig* config);
	void Run();
};


