#pragma once
#include <cstdint>


struct VideoEncoderFrameConfig {
    // Force specific slice as intra
    bool* forceIntraSliceArray;

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
    void* SEIData;

    // SEI data size
    int         SEISize;

    // Per frame flags (VEncFrameFlags)
    int         flags;

    // Frame timestamp in us
    uint64_t    timestamp;

    // Frame number
    int         frameNumber;

	//Saved origin RVR::VEncFrameConfig* for being passed back to RVR SDK 
    void* originPointer;
};
