/*----------------------------------------------------------------------------*\
**          Copyright (c) 2017-2019 Qualcomm Technologies, Inc.               **
**                       All Rights Reserved.                                 **
**     Confidential and Proprietary - Qualcomm Technologies, Inc.             **
\*----------------------------------------------------------------------------*/

#pragma once
#include <d3d11.h>
#include <stdint.h>

 
#define picoModel
struct EncodeOutWithInfo
{
	uint64_t encodEnd;
	int autoRateFlag;
	int bitRate;
    int index;
    uint8_t PloadTpye;
};
typedef int (*PushEncodedFrameFun)(char*, int, int, EncodeOutWithInfo);///char buf[],int buflen,int index
typedef int (*LogFun)(const char*, int);///char buf[],int buflen 
namespace RVR
{

//******************************************************************************
#ifdef VENCPLUGIN_EXPORTS
#define VENCPLUGIN_API __declspec(dllexport)
#else
#define VENCPLUGIN_API __declspec(dllimport)
#endif

//******************************************************************************
// IVEncPlugin interface version 1.1
#define IVENCPLUGIN_VER MAKE_VERSION(1, 1)
#define IVENCPLUGIN_VER_STR MAKE_VERSION_STR(1, 1)

//******************************************************************************
enum VEncFrameFlags {
    // End of stream flag is set for the last sample
    VENC_END_OF_STREAM = (1 << 0),

    // Force this frame as intra frame
    VENC_FORCE_INTRA = (1 << 1),

    // Force this frame as IDR
    VENC_FORCE_IDR = (1 << 2),

    // Force specified slices as intra
    VENC_FORCE_INTRA_SLICE = (1 << 3),

    // Request to prepend codec specific data (like SPS/PPS)
    VENC_PREPEND_CSD = (1 << 4),

    // Flag to insert provided SEI data as SEI NAL
    VENC_INSERT_SEI = (1 << 5),

    // Flag to update the GOP
    VENC_GOP_UPDATE = (1 << 6),

    // Flag to update the resolution
    VENC_RESOLUTION_UPDATE = (1 << 7),
    // Flag to update the bitrate
    VENC_BITRATE_UPDATE = (1 << 8),
    VENC_BITRATE_UPDATE_BY_USER = (1 << 9),
    VENC_BITRATE_UPDATE_BY_CONFIG = (1 << 10),
    VENC_BITRATE_UPDATE_BY_NET = (1 << 11)
};

//******************************************************************************
struct VEncFrameConfig {
    // Force specific slice as intra
    bool*       forceIntraSliceArray;

    // ForceIntraSlice array size
    int         forceIntraSliceArraySize;

    // Start intra refresh for specified number of frames
    // Need to make sure VEncConfig::enableForceIntraRefresh  is enabled
    int         forceIntraRefreshFrameCount;

    // Video avg bitrate
    int         avgBitRate;

    // Video max bitrate
    int         maxBitRate;

    // Image width
    int         width;

    // Image height
    int         height;

    // SEI data to be embedded
    void*       SEIData;

    // SEI data size
    int         SEISize;

    // Per frame flags (VEncFrameFlags)
    int         flags;

    // Frame timestamp in us
    uint64_t    timestamp;

    // Frame number
    int         frameNumber;
    int         net_cost;
};

//******************************************************************************
enum VEncFlags {
    // Flag to enable hardware acceleration
    VENC_ENABLE_HW_ACCEL = (1 << 0)
};

// Forward declaration
class IEncOutputSink;

//******************************************************************************
struct VEncConfig {
    // Instance identifier; cookie used differentiate encoder instances
    int instanceId;

    // D3D11 device pointer
    ID3D11Device*   D3D11Device;

    // Encoder output sink to be used for output callback
    IEncOutputSink* encOutputSink;

    // Codec level
    // 40 = Level_4, 41 = Level_4.1, 42 = Level_4.2,
    // 50 = Level_5, 51 = Level_5.1, 52 = Level_5.2,
    // 60 = Level_6, 61 = Level_6.1, 62 = Level_6.2,
    int level;

    // Codec profile
    // 0 = H264.Base, 1 = H264.Main, 2 = H264.High, 3 = H264.Lossless
    // 0 = H265.Main, 1 = H265.MAIN10, 2 = H265.FREXT
    int profile;

    // Entropy coding mode (0 - CAVLC, 1 - CABAC)
    int entropyCodingMode;

    // Image width
    int width;

    // Image height
    int height;

    // Frames per second
    int fps;

    // Format: (0 = BGRA32, 1 = NV12, 2 = P010, 3 = P016)
    int format;

    // I frame interval
    int GOPSize;

    // Enables on demand intra refresh mode
    bool enableForceIntraRefresh;

    // Number of IntraRefresh duration
    // It should be less than intraRefreshPeriod
    int intraRefreshLength;

    // Period between successive IntraRefresh (in number of frames)
    int intraRefreshPeriod;

    // Codec specific data (SPS/PPS) repeation period in frames
    int CSDPeriod;

    // Max reference frames in cc (Decoded picture buffering)
    int maxDPBSize;

    // Video avg bitrate
    int avgBitRate;

    // Video max bitrate
    int maxBitRate;

    // VBV buffer size (if -1, it lets encoder use default value)
    int bufferSizeVBV;

    // Rate control mode ( 0 = CBR, 1 = VBR)
    int rcMode;

    // Codec to be used (0 = h264)
    int codec;

    // Min quantization parameter (for I and P frames)
    int minIQP;
    int minPQP;

    // Max quantization parameter (for I and P frames)
    int maxIQP;
    int maxPQP;

    // Slice mode
    // 0 - slice size is specified in macroblocks/CTUs per slice
    // 1 - slice size is specified in bytes per slice
    // 2 - slice size is specified in units of macroblock/CTU rows per slice
    int sliceMode;

    // Slice size value is going to be in different units
    // based on sliceMode
    int sliceSize;

    // Each frame needs to be encoded independent of other slices in the frame
    bool independentSliceEncode;

    // Flags can be bitwise OR of one or more VEncFlags
    int flags;

    // Preferred encoder output mode
    // 0 - slice based output
    // 1 - frame based output
    int encoderOutputMode;

    //low latency mode, 0 disabled, 1 enabled
    bool lowLatencyMode;
};

//******************************************************************************
interface  IVEncPlugin {
    // After encoder instance is created, RVRPlugin calls Initialize with
    // VEncConfig which has all the encoding parameters for that instance
    virtual HRESULT Initialize(VEncConfig* encConfig) = 0;
	
    virtual HRESULT StopLoop() = 0;
    //virtual HRESULT InitializeRgbVersion(VEncConfig* encConfig) = 0;
    // After encoding session ends, RVRPlugin calls Uninitialize
    virtual HRESULT Uninitialize() = 0;

    // Every new frame to be encoded is queued to encoder using QueueBuffer
    // along with per frame encoding configurations. The same frameConfig
    // pointer has to be passed to IEncOutputSink APIs
    virtual HRESULT QueueBuffer(ID3D11Texture2D* inputBuffer,
        VEncFrameConfig* frameConfig) = 0;

    // Flush is called by RVRPluin to at the end of session to make sure all
    // the queued samples are processed and get ready for Uninitialize
    virtual HRESULT Flush() = 0;
	 
    // Destructor
    virtual ~IVEncPlugin() {};

	
};

//******************************************************************************
extern "C" {
    // To be exported by VideoEncoder plugin to create encoder instance
    VENCPLUGIN_API IVEncPlugin* CreateVEncPlugin();
	VENCPLUGIN_API IVEncPlugin* CreateVEncPluginPico();
    // Return IVEncPlugin interface version number
    // Major & Minor versions each 16bits
    // UINT32 Version = ((MajorVersion << 16) | MinorVersion)
    VENCPLUGIN_API UINT32 GetIVEncPluginVersion();

    // Return VEncPlugin implementation name
    VENCPLUGIN_API CHAR* GetVEncPluginName();

	VENCPLUGIN_API void SetPoseCache(void *pose);
    VENCPLUGIN_API void SetPoseDepth(float depth);
	VENCPLUGIN_API void SetIp(char *dstip, int len);
    VENCPLUGIN_API void RegistPushEncodedFrameFun(PushEncodedFrameFun  funptr);
    VENCPLUGIN_API void RegistLogFun(LogFun  funptr);
}

typedef IVEncPlugin* (WINAPI* CREATE_VENC_PLUGIN)();

typedef UINT32 (WINAPI* GET_IVENCPLUGIN_VERSION)();

typedef CHAR* (WINAPI* GET_VENCPLUGIN_NAME)();

} // Namespace RVR