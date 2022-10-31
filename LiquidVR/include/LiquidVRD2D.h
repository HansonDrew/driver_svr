//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once
#include "LiquidVR.h"

enum ALVR_DISPLAY_COLOR_DEPTH
{
    ALVR_DISPLAY_COLOR_DEPTH_666               = 0,
    ALVR_DISPLAY_COLOR_DEPTH_888               = 1,
    ALVR_DISPLAY_COLOR_DEPTH_101010            = 2,
};

enum ALVR_DISPLAY_PIXEL_ENCODING
{
    ALVR_DISPLAY_PIXEL_ENCODING_RGB            = 0,
    ALVR_DISPLAY_PIXEL_ENCODING_YCbCr422       = 1,
    ALVR_DISPLAY_PIXEL_ENCODING_YCbCr444       = 2,
};

struct ALVRDisplayTiming
{
    unsigned int    active;
    unsigned int    blank;
    unsigned int    syncOffset;
    unsigned int    syncWidth;
    bool            positiveSyncPolarity;
};

// For future use
enum ALVR_DISPLAY_MODE_FLAGS
{
    ALVR_DISPLAY_MODE_DEFAULT               = 0x00000000,
    ALVR_DISPLAY_MODE_DITHER_DISABLE        = 0x00000001,
    ALVR_DISPLAY_MODE_PROCESSING_BYPASS     = 0x00000002,
};

struct ALVRDisplayMode
{
    ALVRSize2D sourceSize;

    struct
    {
        unsigned long long  pixelClockInKhz;
        ALVRDisplayTiming   horizontal;
        ALVRDisplayTiming   vertical;
    } timing;

    ALVR_DISPLAY_COLOR_DEPTH    colorDepth;
    ALVR_DISPLAY_PIXEL_ENCODING pixelEncoding;

    struct
    {
        bool        scalingEnabled;
        ALVRSize2D  destinationSize;
        ALVRPoint2D offset;
    } scaling;

    unsigned int    flags;         // Reserved
};

struct ALVRDisplayAdapterInfo
{
    LUID         adapterLuid;
    unsigned int gpuIdx;
};

enum ALVR_DISPLAY_CONNECTOR_TYPE
{
    ALVR_DISPLAY_CONNECTOR_UNKNOWN  = 0,
    ALVR_DISPLAY_CONNECTOR_VGA      = 1,
    ALVR_DISPLAY_CONNECTOR_DVI_D    = 2,
    ALVR_DISPLAY_CONNECTOR_DVI_I    = 3,
    ALVR_DISPLAY_CONNECTOR_HDMI     = 4,
    ALVR_DISPLAY_CONNECTOR_DP       = 5,
    ALVR_DISPLAY_CONNECTOR_EDP      = 6,
    ALVR_DISPLAY_CONNECTOR_MINI_DP  = 7,
};

enum ALVR_DISPLAY_BRIDGE_TYPE
{
    ALVR_DISPLAY_BRIDGE_UNKNOWN                              = 0,
    ALVR_DISPLAY_BRIDGE_DONGLE                               = 1,
    ALVR_DISPLAY_BRIDGE_DONGLE_PASSIVE_DP_DVI_SINGLELINK     = 2,
    ALVR_DISPLAY_BRIDGE_DONGLE_PASSIVE_DP_HDMI               = 3,
    ALVR_DISPLAY_BRIDGE_DONGLE_ACTIVE_DP_VGA                 = 4,
    ALVR_DISPLAY_BRIDGE_DONGLE_ACTIVE_DP_DVI_SINGLE_LINK     = 5,
    ALVR_DISPLAY_BRIDGE_DONGLE_ACTIVE_DP_DVI_DUAL_LINK       = 6,
    ALVR_DISPLAY_BRIDGE_DONGLE_ACTIVE_DP_HDMI                = 7,
    ALVR_DISPLAY_BRIDGE_HUB                                  = 8
};

#define ALVR_BRIDGE_IDENTIFIER_SIZE 16

struct ALVRConnectionBridge
{
    ALVR_DISPLAY_BRIDGE_TYPE type;
    unsigned char identifier[ALVR_BRIDGE_IDENTIFIER_SIZE];
};

enum ALVR_DISPLAY_VISIBILITY
{
    ALVR_DISPLAY_VISIBILITY_PUBLIC = 0,
    ALVR_DISPLAY_VISIBILITY_TEMPORARY_HIDDEN = 1,
    ALVR_DISPLAY_VISIBILITY_PERMANENTLY_HIDDEN = 2,
};


ALVR_INTERFACE("FA508FF7-344E-4CA9-A6F3-4CF34501356F")
ALVRDisplayInfo : public ALVRInterface
{
    virtual uint64_t        ALVR_STD_CALL               GetSessionId(void) const = 0;
    virtual ALVR_RESULT     ALVR_STD_CALL               GetSupportedFormats(size_t* pFormatCount, ALVR_FORMAT* pFormats) const = 0;
    virtual ALVR_RESULT     ALVR_STD_CALL               GetDisplayId(size_t* pDataSize, void* pData) const = 0;
    virtual ALVR_RESULT     ALVR_STD_CALL               GetAdapterInfo(ALVRDisplayAdapterInfo* pAdapterInfo) const = 0;
    virtual ALVR_RESULT     ALVR_STD_CALL               GetConnectorInfo(ALVR_DISPLAY_CONNECTOR_TYPE* pConnectorType,
                                                                         size_t* pBridgeCount,
                                                                         ALVRConnectionBridge* pConnectorInfo) const = 0;
    virtual unsigned int    ALVR_STD_CALL               GetTargetId(void) const = 0;
    virtual const wchar_t*  ALVR_STD_CALL               GetName(void) const = 0;
    virtual ALVR_DISPLAY_VISIBILITY     ALVR_STD_CALL   GetVisibility(void) const = 0;
    virtual unsigned short  ALVR_STD_CALL               GetManufacturerName(void) const = 0;
    virtual unsigned short  ALVR_STD_CALL               GetProductCode(void) const = 0;
};



/**
***************************************************************************************************
* @brief Display interface - represents one display
***************************************************************************************************
*/

enum ALVR_DISPLAY_POWER_MANAGEMENT_FLAGS
{
    ALVR_DISPLAY_OS_POWER_MANAGEMENT_DISABLED   = 0x00000000,
    ALVR_DISPLAY_USE_OS_POWER_MANAGEMENT        = 0x00000001,
};

struct ALVRDisplayOpenDesc
{
    unsigned int        powerMgtFlags;  // ALVR_DISPLAY_POWER_MANAGEMENT_FLAGS
};

enum ALVR_DISPLAY_BLANK_MODE
{
    ALVR_DISPLAY_BLANK_DEFAULT       = 0,
    ALVR_DISPLAY_BLANK_BACKLIGHT_OFF = 1,
};

enum ALVR_DISPLAY_POWER_MODE
{
    ALVR_DISPLAY_POWER_OFF           = 0,
    ALVR_DISPLAY_POWER_ON            = 1,
};

enum ALVR_DISPLAY_PRESENT_FLAGS
{
    ALVR_DISPLAY_PRESENT_VSYNC                  = 0x00000001,
};

struct ALVRDisplayPresentStats
{
    uint64_t            presentCount;
    uint64_t            presentCpuTime;         // QPC:  return of QueryPerformanceCounter()
    uint64_t            vsyncCount;
    uint64_t            vsyncCpuTime;           // QPC:  return of QueryPerformanceCounter()
    uint64_t            displayLatchedCpuTime;
    uint64_t            vsyncGpuTime;
};

struct ALVRPresentableSurfaceDesc
{
    ALVR_FORMAT         presentableFormat;
    unsigned int        surfaceFlags;   	// ALVR_SURFACE_FLAGS
    unsigned int        apiSupport;     	// ALVR_RESOURCE_API_SUPPORT
    ALVR_FORMAT         shaderInputFormat; 	// Optional, can be defined by format, put ALVR_FORMAT_UNKNOWN (0) if not needed
    ALVR_FORMAT         shaderOutputFormat; // Optional, can be defined by format, put ALVR_FORMAT_UNKNOWN (0) if not needed
};

#define ALVR_MAX_GAMMA_RAMP_CONTROL_POINTS 1025

struct ALVR_RGB_FLOAT
{
    float red;
    float green;
    float blue;
};

struct ALVRDisplayGammaRamp
{
    ALVR_RGB_FLOAT gammaCurve[ALVR_MAX_GAMMA_RAMP_CONTROL_POINTS];
};


struct ALVRMatrix3x4
{
#pragma pack(push, 1)
    union
    {
        struct 
        {
            float xr, xg, xb, xk;
            float yr, yg, yb, yk;
            float zr, zg, zb, zk;
        } s;            //  Matrix structure representation
        float a[3][4];  //  Two-dimensional array representation
        float v[12];    //  Single-dimensional array representation
    };
#pragma pack(pop)
};

enum ALVR_HDCP_STATUS
{
    ALVR_HDCP_OFF   = 0,
    ALVR_HDCP_ON    = 1,
};

struct ALVRDisplayColorTransform
{
    ALVRMatrix3x4 transform;
};

struct ALVRDummyDisplayDesc
{
    size_t          displayIdSize;
    const void*     pDisplayId;
    ALVRSize2D      displaySize;
    ALVRRational    refreshRate;
};

ALVR_INTERFACE("8FCC95E7-B938-4DDD-95A7-2ABE83813DC4")
ALVRDisplay : public ALVRPropertyStorage
{
    virtual ALVRDisplayInfo*    ALVR_STD_CALL   GetInfo(void) const = 0;

    virtual ALVR_RESULT         ALVR_STD_CALL   Open(ALVRDeviceEx* pDevice, const ALVRDisplayOpenDesc* pDesc) = 0;
    virtual ALVR_RESULT         ALVR_STD_CALL   Close(void) = 0;

    virtual ALVR_RESULT         ALVR_STD_CALL   GetDisplayMode(ALVRDisplayMode* pMode) = 0;
    virtual ALVR_RESULT         ALVR_STD_CALL   SetDisplayMode(const ALVRDisplayMode* pMode) = 0;

    virtual ALVR_RESULT         ALVR_STD_CALL   CreatePresentableSurface(
        const ALVRPresentableSurfaceDesc* pDesc,
        ALVRSurface** ppSurf) = 0;

    virtual ALVR_RESULT         ALVR_STD_CALL   Blank(ALVR_DISPLAY_BLANK_MODE blankMode) = 0;
    virtual ALVR_RESULT         ALVR_STD_CALL   Present(
                                    ALVRSurface* pSurf,
                                    unsigned int presentFlags,  // ALVR_DISPLAY_PRESENT_FLAGS
                                    ALVRFence* pFence) = 0;

    virtual ALVR_RESULT         ALVR_STD_CALL   GetPresentStats(ALVRDisplayPresentStats* pStats) = 0;
    virtual ALVR_RESULT         ALVR_STD_CALL   GetScanLine(int* pScanLine) = 0;

    virtual ALVR_RESULT         ALVR_STD_CALL   SetPowerMode(ALVR_DISPLAY_POWER_MODE powerMode) = 0;
    virtual ALVR_RESULT         ALVR_STD_CALL   SetGammaRamp(const ALVRDisplayGammaRamp* pGammaRamp) = 0;
    virtual ALVR_RESULT         ALVR_STD_CALL   SetColorTransform(const ALVRDisplayColorTransform* pColorTransform) = 0;

    virtual ALVR_RESULT         ALVR_STD_CALL   SetOsEventOnVsync(HANDLE hEvent, unsigned int usDelay, bool repeated) = 0; // usDelay in microseconds, don't set more then VSYNC time
    virtual ALVR_RESULT         ALVR_STD_CALL   SignalSemaphoreOnVsync(ALVRGpuSemaphore* pSemaphore, unsigned int usDelay) = 0; // usDelay in microseconds, don't set more then VSYNC time

    virtual ALVR_RESULT         ALVR_STD_CALL   GetHdcpStatus(ALVR_HDCP_STATUS* pStatus) = 0;
};

/**
***************************************************************************************************
* @brief Display notification interface - callback
***************************************************************************************************
*/

ALVR_INTERFACE("48E5248F-AB73-42A2-84B1-4CCC78C85319")
ALVRDisplayNotification : public ALVRInterface
{
    virtual ALVR_RESULT     ALVR_STD_CALL   ChangedTopology(void) = 0;
};

/**
***************************************************************************************************
* @brief Display enumerator interface
***************************************************************************************************
*/
ALVR_INTERFACE("99405DF7-B785-4706-BE9B-3D4DE0D3FDC6")
ALVRDisplayInfoEnumerator : public ALVRInterface
{
    virtual ALVR_RESULT     ALVR_STD_CALL   Reset() = 0;
    virtual ALVR_RESULT     ALVR_STD_CALL   Next(ALVRDisplayInfo** ppDisplayInfo) = 0;
};


enum ALVR_DISPLAY_FLAGS
{
    ALVR_DISPLAY_NONE = 0x00000000,
    ALVR_DISPLAY_ENABLE_AUDIO = 0x00000001,
};

ALVR_INTERFACE("983CA36D-42C1-44DD-BDDB-8A518B2E7392")
ALVRDisplayManager : public ALVRPropertyStorage
{
    virtual ALVR_RESULT     ALVR_STD_CALL   RegisterNotification(ALVRDisplayNotification* pCallback) = 0;

    virtual ALVR_RESULT     ALVR_STD_CALL   EnumerateDisplays(ALVR_DISPLAY_VISIBILITY visibility, ALVRDisplayInfoEnumerator** ppDisplayInfoEnum) = 0;
    virtual ALVR_RESULT     ALVR_STD_CALL   CreateHiddenDisplay(uint64_t sessionId, ALVRDisplay** ppDisplay) = 0;
    virtual ALVR_RESULT     ALVR_STD_CALL   CreateDebugDisplay(HWND hWnd, bool bFullScreen, ALVRDisplay** ppDisplay) = 0;

    // Permanent display management
    virtual ALVR_RESULT     ALVR_STD_CALL   RegisterHiddenDisplay(unsigned short manufacturerName, unsigned short productCodeMin, unsigned short productCodeMax) = 0;
    virtual ALVR_RESULT     ALVR_STD_CALL   UnregisterHiddenDisplay(unsigned short manufacturerName, unsigned short productCodeMin, unsigned short productCodeMax) = 0;

    // Temporary display management
    virtual ALVR_RESULT     ALVR_STD_CALL   HideDisplay(uint64_t sessionId) = 0;
    virtual ALVR_RESULT     ALVR_STD_CALL   ShowDisplay(uint64_t sessionId) = 0;

    // Dummy display management
    virtual ALVR_RESULT     ALVR_STD_CALL   AddDummyDisplay(const ALVRDummyDisplayDesc *pDesc, uint64_t* pSessionId) = 0;
    virtual ALVR_RESULT     ALVR_STD_CALL   RemoveDummyDisplay(uint64_t sessionId) = 0;

    // Extended method
    virtual ALVR_RESULT     ALVR_STD_CALL   RegisterHiddenDisplayEx(
        unsigned short manufacturerName, unsigned short productCodeMin, unsigned short productCodeMax, 
            unsigned int flags) = 0;    // ALVR_DISPLAY_FLAGS

};

/**
***************************************************************************************************
* @brief Power callback interface
***************************************************************************************************
*/

enum ALVR_POWER_PROFILE
{
    ALVR_POWER_PROFILE_DEFAULT      = 0,
    ALVR_POWER_PROFILE_VR_CUSTOM    = 1,
    ALVR_POWER_PROFILE_VR_DEFAULT   = 2,
};

/**
***************************************************************************************************
* @brief Power regions if ALVR_POWER_PROFILE_VR_CUSTOM is in effect
***************************************************************************************************
*/

#define ALVR_MAX_POWER_FRAME_REGIONS 5

struct ALVRPowerRegion
{
	uint64_t timeBegin;    // in us, relative to the frame start at V-sync
	uint64_t timeEnd;      // in us, relative to the frame start at V-sync
	float    performance;  // 0..1 scale for min to max performance
};

enum ALVR_POWER_REGION_FLAGS
{
	ALVR_POWER_REGION_REPEATED = 0x00000000,
};

struct ALVRPowerFrameDesc
{
	ALVRDisplay* pDisplay;		// Power description will be applied to this display
	unsigned int regionCount;	// Maximum is ALVR_MAX_POWER_FRAME_REGIONS
	const ALVRPowerRegion* pRegions;
	unsigned int flags;         // ALVR_POWER_REGION_FLAGS
};

/**
***************************************************************************************************
* @brief Power manager interface
***************************************************************************************************
*/

ALVR_INTERFACE("706213E6-0CBC-4160-BE97-879DACB296F5")
ALVRPowerManager : public ALVRPropertyStorage
{
    virtual ALVR_RESULT     ALVR_STD_CALL   SetPowerProfile(ALVR_POWER_PROFILE powerProfile) = 0;
    virtual ALVR_RESULT     ALVR_STD_CALL   SetCustomMinPerformance(float performance,
        float* pActualPerformance) = 0;     // Ptr to one float element
    virtual ALVR_RESULT     ALVR_STD_CALL   SetCustomPowerRegions(const ALVRPowerFrameDesc* pFrame,
        float* pActualPerformance) = 0;     // Ptr to float array containing pFrame->regionCount elements
};



/**
***************************************************************************************************
* @brief Display factory interface
***************************************************************************************************
*/

ALVR_INTERFACE("D4FFD185-A3EE-4B93-A44A-720265282CAA")
ALVRDisplayFactory
{
    virtual ALVR_RESULT     ALVR_STD_CALL   CreateManager(
                                    void* pConfigDesc, // For future use
                                    ALVRDisplayManager** ppManager) = 0;

    virtual ALVR_RESULT     ALVR_STD_CALL   CreatePowerManager(
                                    ALVRDeviceEx* pDevice,
                                    void* pConfigDesc, // For future use
                                    ALVRPowerManager** ppManager) = 0;
};


#define ALVR_DISPLAY_INIT_FUNCTION_NAME    "ALVRDisplayInit"

/**
***************************************************************************************************
* @brief Display entry point
***************************************************************************************************
*/
typedef ALVR_RESULT(ALVR_CDECL_CALL *ALVRDisplayInit_Fn)(uint64_t version, void** ppFactory);

/**
***************************************************************************************************
* End of file
***************************************************************************************************
*/

