/*----------------------------------------------------------------------------*\
**          Copyright (c) 2017-2019 Qualcomm Technologies, Inc.               **
**                       All Rights Reserved.                                 **
**     Confidential and Proprietary - Qualcomm Technologies, Inc.             **
\*----------------------------------------------------------------------------*/
#pragma once

#include <stdint.h>

#define OUTBUFSIZE 200
#define POSELISTSIZE 100
namespace RVR
{

//******************************************************************************
// If modified, please update managed enum as well
enum class RVRAudioMode
{
    None,
    File,
    Stream
};

//******************************************************************************
// If modified, please update managed enum as well
enum class RVRPoseMode {
    LocalPoseFromMouse,
    LocalPoseFromFile,
    RemotePoseFromHMD,
};

//******************************************************************************
// If modified, please update managed enum as well
enum class RVRPluginEvent
{
    None = 0,
};

//******************************************************************************
// If modified, please update managed enum as well
enum class RVRState
{
    Disable,
    Enable,
};

//******************************************************************************
// If modified, please update managed enum as well
enum class RVREyeMask
{
    Left  = 0x1,
    Right = 0x2,
    Both  = 0x3
}; 

//******************************************************************************
enum class RVRControllerConnectionState {
    kNotInitialized = 0,
    kDisconnected = 1,
    kConnected = 2,
    kConnecting = 3,
    kError = 4
};

//******************************************************************************
enum RVRConfigUpdateFlag
{
    UPDATE_BITRATE = (1 << 0),
    FORCE_INTRA = (1 << 1)
};

//******************************************************************************
enum class RVREvent
{
    RVR_EVENT_NONE = 0,
    RVR_EVENT_VIBRATION,
    RVR_EVENT_RECENTER,
};
//******************************************************************************
enum class RVREventRecenterMask
{
    RVR_EVENT_RECENTER_NONE = 0,
    RVR_EVENT_RECENTER_CONTROLLER = 1,
    RVR_EVENT_RECENTER_HMD = 2,
};

//******************************************************************************
// If modified, please update managed struct as well
struct PluginSettings
{
    int                 width;
    int                 height;
    int                 fps;
    float               fov;
    float               nearClipPlane;
    float               farClipPlane;

    RVRState            localRendering;
    RVRPoseMode         poseMode;
    char                poseFeederInputFile[512];
    char                poseFeederOutputFile[512];
    char                controllerPoseOutputFile[512];

    float               interPupilDistance;
    float               initialOffsetX;
    float               initialOffsetY;
    float               initialOffsetZ;
    float               initialOffsetPitch;
    float               initialOffsetYaw;
    float               initialOffsetRoll;
    float               positionScaleX;
    float               positionScaleY;
    float               positionScaleZ;

    RVRState            transparentBackground;

    RVRState            AAFmode;
    float               minThreshhold;
    float               edgeThreshhold;
    float               edgeSharpness;

    RVRAudioMode        audioMode;

    int                 videoPortL;
    int                 videoPortR;
    int                 mtu;
    int                 poseTransmiterPort;
    int                 debugOverlay;
    char                sinkType[5];
    int                 audioPort;
    int                 logLevel;

	int                 streamingWidth  {0};
	int                 streamingHeight {0};
    int                 eventPort;
    // Padding section
    unsigned long       reserve1;
};
static_assert(sizeof(PluginSettings) == 1684, "Size needs to be maintained for backwards compatibility");


//******************************************************************************
struct RVRConfigUpdate
{
    int flags; //Per frame flags (RVRConfigUpdateFlag)
    int avgBitRate; // Video avg bitrate
    int maxBitRate; // Video max bitrate
};

//******************************************************************************
struct RVRStats
{
    bool isConnected; // Remote connection status; stats are valid only in connected state
    float avgM2R2P; // average end to end latency
    float avgPER; // average packet error rate
    float framePER; //packer error for current frame
};

//******************************************************************************
struct RVRQuaternion
{
    float x, y, z, w;
};
struct RVRVector3
{
    float x, y, z;
};
struct RVRVector2
{
    float x, y;
};
 
//******************************************************************************
struct RVRPoseHmdData
{
    bool valid;
    RVRQuaternion rotation;
    RVRVector3 position;

    uint64_t poseTimeStamp;
    float predictedTimeMs;
    uint64_t poseRecvTime;
    RVRVector3 linearVelocity;
    RVRVector3 linearAcceleration;
    RVRVector3 angularVelocity;
    RVRVector3 angularAcceleration;
    uint64_t hmdTimeStamp;
    uint64_t beginGameRenderStamp;
    uint64_t endGameRenderStamp;
    uint64_t beginEncodeStamp;
    uint64_t endEncodeStamp;
};


//******************************************************************************
struct RVRControllerData
{
    RVRQuaternion rotation;
    RVRVector3 position;
    RVRVector3 vecAngularAcceleration;
    RVRVector3 vecAcceleration;
    uint32_t buttonState;
    RVRVector2 analog2D[4];
    float analog1D[8];
    uint32_t isTouching;
    RVRControllerConnectionState connectionState;
    int64_t timestamp;
    RVRVector3 vecVelocity;
    RVRVector3 vecAngularVelocity;
    uint64_t recvTimeStampNs;
};

struct RVREventVibration
{
    uint32_t eventType;
    int32_t controller_index;
    float durationSeconds;
    float frequency;
    float amplitude;
};
//
//struct VEncFrameConfig {
//	// Force specific slice as intra
//	bool*       forceIntraSliceArray;
//
//	// ForceIntraSlice array size
//	int         forceIntraSliceArraySize;
//
//	// Start intra refresh for specified number of frames
//	// Need to make sure VEncConfig::enableForceIntraRefresh  is enabled
//	int         forceIntraRefreshFrameCount;
//
//	// Video avg bitrate
//	int         avgBitRate;
//
//	// Video max bitrate
//	int         maxBitRate;
//
//	// Image width
//	int         width;
//
//	// Image height
//	int         height;
//
//	// SEI data to be embedded
//	void*       SEIData;
//
//	// SEI data size
//	int         SEISize;
//
//	// Per frame flags (VEncFrameFlags)
//	int         flags;
//
//	// Frame timestamp in us
//	uint64_t    timestamp;
//
//	// Frame number
//	int         frameNumber;
//};

struct RVREventRecenter
{
    uint32_t eventType;
    uint32_t what;
};

}; // Namespace RVR
