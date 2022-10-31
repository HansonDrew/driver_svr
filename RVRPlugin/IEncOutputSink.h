/*----------------------------------------------------------------------------*\
**          Copyright (c) 2017-2018 Qualcomm Technologies, Inc.               **
**                       All Rights Reserved.                                 **
**     Confidential and Proprietary - Qualcomm Technologies, Inc.             **
\*----------------------------------------------------------------------------*/

#pragma once
#include <stdint.h>

#include "IVEncPlugin.h"
 
namespace RVR
{

// Forward declaration
//struct VEncFrameConfig;

//******************************************************************************
/// Interface to be used by VEncPlugin to give a callback when the output frame
/// is ready or a slice of the frame is ready
/// VEncPlugin may provide sliceOffsets within output buffer (including NAL
/// start code prefix), if it is readily available else it can be NULL
class IEncOutputSink {
public:
    virtual size_t ProcessSample(const void* buffer, size_t bufferLength,
        RVR::VEncFrameConfig* frameConfig,
        uint32_t* sliceOffsets, uint32_t numOfSlices) = 0;

    virtual size_t ProcessSlice(const void* buffer, size_t bufferLength,
        RVR::VEncFrameConfig* frameConfig, bool lastSlice) = 0;
};

}