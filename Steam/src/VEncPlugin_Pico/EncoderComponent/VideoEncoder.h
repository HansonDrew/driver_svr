#pragma once


#include <d3d11.h>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "VideoEncoderConfig.h"
#include "../RVRPlugin/RVRPluginDefinitions.h"
struct VideoEncoderFrameConfig;


#define MSG_ENCODE WM_USER+104
#define MSG_PREENCODE WM_USER+103
#define  MSG_PACKET_AND_SEND WM_USER+106
#define MSG_PREPACKET_AND_SEND WM_USER+105

#define MSG_EXIT WM_USER+109
#define SHARETEXTNUM 3
 using namespace std;
typedef struct _OutFrame
{
	uint8_t* buf;
	atomic_int len;
	atomic_ullong index;
}OutFrame;

class VideoEncoder
{
public:
	virtual void Initialize(const VideoEncoderConfig& config) = 0;
	virtual void Reconfigure(const VideoEncoderConfig& config) = 0;
	
	virtual void Transmit(ID3D11Texture2D *pTexture, VideoEncoderFrameConfig* frameConfig) = 0;

	virtual void Flush() = 0;
	
	virtual void Shutdown() = 0;

	virtual void SetBitRate(VideoEncoderFrameConfig* frameConfig) = 0;
	void SetRtpThreadId(int thread_id) { rtp_thread_id_ = thread_id; }
	virtual bool SupportsReferenceFrameInvalidation() = 0;
	virtual void InvalidateReferenceFrame(uint64_t videoFrameIndex) = 0;
	void CreateOutFrameBufferAndInitParts();
	void DeleteOutFrameBuffer();
	FILE* pleft = NULL;
	FILE* pright = NULL;
	bool spspps_start_ = false;

	//HANDLE mHThreadEvent;
	int mIndex;
	bool mEncoderRun;
	int rtp_thread_id_;
	OutFrame mOutFrame[OUTBUFSIZE];
	uint64_t mRenderCost[POSELISTSIZE];
	atomic_ullong out_frame_index_;
	uint64_t last_produce_time_ns_;
	std::mutex frame_index_mutex_;
	std::condition_variable frame_index_cv_;
	int mNoIDRTime=0;
	bool init_success_ = false;
protected:
};
