//-----------------------------------------------------------------------------
//  Copyright (c) 2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#pragma once
#include <Windows.h>

class AudioCaptureSession
{
public: 
private:
    HANDLE hStopThreadEvent;
    HANDLE hCaptureThread; 
    int mSamples = 0;
private:
    static DWORD WINAPI audio_capture_session_func(PVOID);
    void ThreadFunc();
public:
   
    AudioCaptureSession();
    ~AudioCaptureSession();
    void Start();
    int GetSample();
    void Stop();
};
