#pragma once
#include "../VideoEncoder.h"

#include "../../amf/public/common/AMFFactory.h"
#include "../../amf/public/include/components/VideoEncoderVCE.h"
#include "../../amf/public/include/components/VideoEncoderHEVC.h"
#include "../../amf/public/common/AMFSTL.h"
#include "../../amf/public/common/Thread.h"

#include "../AsyncDataProcessor.h"

#include "AMFTextureEncoder.h"


// Video encoder for AMD VCE.
class VideoEncoderVCE : public VideoEncoder
{
public:
	VideoEncoderVCE();
	~VideoEncoderVCE();
	int idr_count_ = 0;
	void Initialize(const VideoEncoderConfig& config) override;
	void Reconfigure(const VideoEncoderConfig& config) override;
	void Shutdown() override;
	void Flush() override;
	void Transmit(ID3D11Texture2D* pTexture, VideoEncoderFrameConfig* frameConfig) override;
	bool SupportsReferenceFrameInvalidation() override { return false; };
	void InvalidateReferenceFrame(uint64_t videoFrameIndex) override {};
#define surfacenum 6
	//amf::AMFSurfacePtr surface_buf_[surfacenum] = { nullptr };
	//int surface_index_ = 0;
	//Called from AMFTextureEncoder
	void Receive(amf::AMFData* data);

	//Called from AsyncDataProcessor
	void DeferSend(const void* data, size_t len, void* userData);

private:

	void ApplyFrameType(const amf::AMFSurfacePtr& surface, VideoEncoderFrameConfig* frameConfig);
	inline void ApplyFrameROI(const amf::AMFSurfacePtr& surface, const amf::AMFSurfacePtr& roiSurface);
	void SkipAUD(unsigned char** buffer, int* length);
	amf::AMFSurfacePtr CreateROISurface(int surfaceWidth, int surfaceHeight, int roiLevel);
	int get_idr_resend_count() const { return idr_resend_count_at_startup_; };
	void set_idr_resend_count(int count) { idr_resend_count_at_startup_ = count; };

	static const wchar_t* FRAME_INDEX;
	static const wchar_t* START_TIME_PROPERTY;
	static const wchar_t* VIDEO_FRAME_CONFIG_POINTER_PROPERTY;
	
	const uint64_t MILLISEC_TIME = 10000;
	const uint64_t MICROSEC_TIME = 10;

	amf::AMFContextPtr mContext = nullptr;
	amf::AMFSurfacePtr mRoiSurface = nullptr;
	std::shared_ptr<AMFTextureEncoder> mEncoder;
	std::shared_ptr<AsyncDataProcessor> mSender;
	
	VideoEncoderConfig mConfig;
	ID3D11DeviceContext* contextFromDeviceInConfig = nullptr;

	int idr_resend_count_at_startup_ = 0;
};


