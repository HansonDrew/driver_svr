#pragma once
 
#include <d3d11.h>
#include "../VideoEncoder.h"
#include "NvEncoderD3D11.h"
#include "NvEncoderCLIOptions.h "

class VideoEncoderNVENC :
	public VideoEncoder
{
public:
	ID3D11Texture2D *mEncodeText2DS[5];
	int mEncodeTextInedx = 0;
	VideoEncoderNVENC();
	~VideoEncoderNVENC();
	int idr_count_ = 0;
	void Initialize(const VideoEncoderConfig& config);
	void Reconfigure(const VideoEncoderConfig& config);
	//void InitializeRgbVersion(const VideoEncoderConfig& config);
	uint64_t mEndTime;
	uint64_t mStartTime;
	void Transmit(ID3D11Texture2D *pTexture, VideoEncoderFrameConfig* frameConfig);
	int  mEncoderFlag;
	void Flush();

	void Shutdown();
	static unsigned int __stdcall GetEncodeFrameThread(LPVOID lpParameter);
	void GetEncodedData();
	 
	//static unsigned int __stdcall PacketAndSendThread(LPVOID lpParameter);
	unsigned  mEncodeThreadId;
	unsigned  mEncodeThreadId2;
	static unsigned int __stdcall EncodeThread(LPVOID lpParameter);
	static unsigned int __stdcall EncodeThread2(LPVOID lpParameter);
	
	int mSvrFrameConfigIndex;
	
	void *mVideoConfigBuf[OUTBUFSIZE];
	
	bool SupportsReferenceFrameInvalidation() override { return false; };
	void InvalidateReferenceFrame(uint64_t videoFrameIndex) override {};
	
	void *mSvrFrameConfig[OUTBUFSIZE];

	HANDLE mHEncoderCreateEvent;
	
	uint64_t sendtime = 0;
	float sendcount = 0.000;
	NvEncoderD3D11 *mEncoder;
	
	VideoEncoderConfig mConfig;
	ID3D11DeviceContext* mContextFromDeviceInConfig = nullptr;
	
	//If you need to create your own D3dDevice
	ID3D11Device* mDeviceOfEncoder = nullptr;
	ID3D11Device* mDeviceOfSVR = nullptr;
	ID3D11DeviceContext* mD3D11ContextOfEncoder = nullptr;
	NV_ENC_INITIALIZE_PARAMS mInitializeParams;
protected:
private:
	void CreateDevice(ID3D11Device*& device);
	NvEncoderInitParam*mEncodeCLIOptions;
	ID3D11Texture2D* CreateSharedTexture2D(ID3D11Device* device, DXGI_FORMAT format, int width, int height, int nSampleCount);
	HANDLE CreateSharedResource(ID3D11Texture2D* texture);


};

