#pragma once

#include "../RVRPlugin/IVEncPlugin.h"
#include <string>


class VideoEncoder;

class VEncPluginPico :
	public RVR::IVEncPlugin
{
public:

	HRESULT Initialize(RVR::VEncConfig* encConfig) override;
	HRESULT StopLoop() override;
	HRESULT Uninitialize() override;
	HRESULT QueueBuffer(ID3D11Texture2D* inputBuffer, RVR::VEncFrameConfig* frameConfig) override;
	HRESULT Flush() override;
   
	//HRESULT PoseCache() {};
	VEncPluginPico();
	~VEncPluginPico();
	unsigned  mPacketAndSendThreadId;
	static unsigned int __stdcall PacketAndSendThread(LPVOID lpParameter);
	static unsigned int __stdcall DebugThread(LPVOID lpParameter);
	
	HANDLE mHThreadEvent;
	int mIndex = 0;
	uint64_t mStartTimeUs = 0;

	uint64_t mFrameCount = 0;
	uint64_t mFrameCountSecond = 0;
	uint64_t mLastStatisticsTimestamp = 0;
	uint64_t mStatisticsCount = 0;
	float mAverageFrame=0.f;

private:
	std::string instance_ptr_str_;
	VideoEncoder* encoder;
};
////工程改自高通驱动。虽然代码已经完全不使用高通的代码，但是因为时间问题，加载高通所定义的接口的dll 没有去掉。定义空的假dll加载。
//class VEncPluginPicoNullDllForQualcomm :
//	public RVR::IVEncPlugin
//{
//public:
//	HRESULT Initialize(RVR::VEncConfig* encConfig) override;
//	HRESULT Uninitialize() override;
//	HRESULT QueueBuffer(ID3D11Texture2D* inputBuffer, RVR::VEncFrameConfig* frameConfig) override;
//	HRESULT Flush() override;
//
//	//HRESULT PoseCache() {};
//	VEncPluginPicoNullDllForQualcomm();
//	~VEncPluginPicoNullDllForQualcomm();
//	
//private:
//	std::string instance_ptr_str_;
//	VideoEncoder* encoder;
//};
