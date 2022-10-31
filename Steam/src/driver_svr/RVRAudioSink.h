//-----------------------------------------------------------------------------
//  Copyright (c) 2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#pragma once
#include "AudioCaptureSession.h"
#include "RVRStub.h"

class RVRAudioSink : public AudioCaptureSession
{
private:
    __int64 startTimeUs;
    WAVEFORMATEX* format;
    RVRStub* mStubInstance;
public:
    RVRAudioSink(RVRStub* stubInstance);
    ~RVRAudioSink();
    void Begin(WAVEFORMATEX* format);
   
    WAVEFORMATEX* GetFormat();
    void End();
};
