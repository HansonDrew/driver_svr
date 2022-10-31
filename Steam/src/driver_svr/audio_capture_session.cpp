//-----------------------------------------------------------------------------
//  Copyright (c) 2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------
#include "audio_rtp_packet.h"
#include "audio_capture_session.h"
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <audiopolicy.h>
#include <stdio.h>
#include "config_reader.h"
#ifndef NO_USBBULK
#include "UsbBulkModule.h"
#endif
#include "driverlog.h"
extern ConfigReader gConfigReader;
extern bool g_save_audio;
#define RELEASE(ptr)  \
              if ((ptr) != NULL)  \
                { (ptr)->Release(); (ptr) = NULL; }

// REFERENCE_TIME time units per second and per millisecond
// REFTIMES is 100ns units
#define REFTIMES_PER_SEC  10000000

static REFERENCE_TIME hnsDefaultDevicePeriod(0);
//-------------------------------------------------------------------
DWORD WINAPI AudioCaptureSession::audio_capture_session_func(PVOID param)
//-------------------------------------------------------------------
{
    AudioCaptureSession* acSession = (AudioCaptureSession*)(param);
    if (acSession != nullptr)
    {
        acSession->ThreadFunc();
    }

    return 0;
}

//-------------------------------------------------------------------
static HANDLE CreatePeriodicTimer(int startTimeMs, int periodicTimeMs)
//-------------------------------------------------------------------
{
    HANDLE periodicTimer = CreateWaitableTimer(NULL, FALSE, NULL);
    LARGE_INTEGER liFirstFire;
    liFirstFire.QuadPart = -1 * startTimeMs * 1000 * 10; //REF TIME 100ns
    SetWaitableTimer(periodicTimer, &liFirstFire, periodicTimeMs, NULL, NULL, FALSE);
    return periodicTimer;
}
//-------------------------------------------------------------------
static HANDLE CreatePeriodicTimer2()
//-------------------------------------------------------------------
{
	HANDLE periodicTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	LARGE_INTEGER liFirstFire;
	liFirstFire.QuadPart = -hnsDefaultDevicePeriod / 2; // negative means relative time
	LONG lTimeBetweenFires = (LONG)hnsDefaultDevicePeriod / 2 / (10 * 1000); // convert to milliseconds

	BOOL bOK = SetWaitableTimer(periodicTimer, &liFirstFire, lTimeBetweenFires, NULL, NULL, FALSE);
	return periodicTimer;
}

//------------------------------------------------------------------
static VOID GetDefaultAudioEndpoint(IMMDevice*& pDevice)
//------------------------------------------------------------------
{
    IMMDeviceEnumerator *pEnumerator = NULL;

    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (SUCCEEDED(hr))
    {
        pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    }
    RELEASE(pEnumerator)
}

BOOL AdjustFormatTo16Bits(WAVEFORMATEX *pwfx)
{
	if (gConfigReader.GetAudioPicoModel()!=1)
	{
		return TRUE;
	}
	BOOL bRet(FALSE);

	if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
	{
		pwfx->wFormatTag = WAVE_FORMAT_PCM;
		pwfx->wBitsPerSample = 16;
		pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
		pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;

		bRet = TRUE;
	}
	else if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
	{
		PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
		if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat))
		{
			pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
			pEx->Samples.wValidBitsPerSample = 16;
			pwfx->wBitsPerSample = 16;
			pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
			pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;

			bRet = TRUE;
		}
	}

	return bRet;
}
//------------------------------------------------------------------
static bool InitializeAudioClient(IMMDevice *pDevice, IAudioClient*& pAudioClient, WAVEFORMATEX*& format, UINT32& bufferFrameCount)
//------------------------------------------------------------------
{
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;

    if( !FAILED(pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient)) )
    {
		HRESULT hr = pAudioClient->GetDevicePeriod(&hnsDefaultDevicePeriod, NULL);
        if (FAILED(hr)) 
        {
            DriverLog("pico audio period failed");
            return false;
        } 
        if (!FAILED(pAudioClient->GetMixFormat(&format)))
        {
            
			if (AdjustFormatTo16Bits(format))
			{
				if (!FAILED(pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, hnsRequestedDuration, 0, format, NULL)))
				{
					if (!FAILED(pAudioClient->GetBufferSize(&bufferFrameCount)))
					{
						return true;
                    }
                    else
                    {
                        DriverLog("pico audio period Get bufferSize failed");
                    }
				}
				else
				{
					DriverLog("pico audio period AudioClient init failed");
				}
            }
            else
            {
                DriverLog("pico audio period adjust format failed");
            }
            
        }
    }
    else
    {
        DriverLog("pico audio Activate failed");
    }

    return false;
}

//------------------------------------------------------------------
static void GetCaptureClient(IAudioClient* pAudioClient, IAudioCaptureClient*& pCaptureClient)
//------------------------------------------------------------------
{

}

//------------------------------------------------------------------
AudioCaptureSession::AudioCaptureSession()
//------------------------------------------------------------------
{
 
    hCaptureThread = NULL;
    hStopThreadEvent = ::CreateEvent(NULL, TRUE, FALSE, TEXT("Stop Event"));
	AudioSender::GetInstance()->InitAudioSender(gConfigReader.GetAudioCapturePort(),6);

}
///


//------------------------------------------------------------------
AudioCaptureSession::~AudioCaptureSession()
//------------------------------------------------------------------
{
    Stop();
    ::CloseHandle(hStopThreadEvent);
    hStopThreadEvent = NULL;
}

//------------------------------------------------------------------
void AudioCaptureSession::Start()
//------------------------------------------------------------------
{
    if (hCaptureThread == NULL)
    {
        hCaptureThread = ::CreateThread(NULL, 0, audio_capture_session_func, (PVOID)this, 0, NULL);
    }
}

//------------------------------------------------------------------
void AudioCaptureSession::Stop()
//------------------------------------------------------------------
{
    SetEvent(hStopThreadEvent);
    WaitForSingleObject(hCaptureThread, INFINITE);
    ::CloseHandle(hCaptureThread);
    hCaptureThread = NULL;
    ResetEvent(hStopThreadEvent);
}
FILE* paudio_file = NULL;
//-------------------------------------------------------------------
 
void Process(IAudioCaptureClient *pCaptureClient,int Samples)
//-------------------------------------------------------------------
{
    BYTE *pData;
    DWORD flags;
    UINT32 packetLength = 0;
    UINT32 numFramesAvailable;

    if (!pCaptureClient)
        return;
    
    if (!FAILED(pCaptureClient->GetNextPacketSize(&packetLength)))
    {
        while (packetLength != 0)
        {
            // Get the available data in the shared buffer.
            if ( FAILED(pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL)) )
            {
                break;
            }
            else 
            {
               
				if (gConfigReader.GetAudioPicoModel()==1)
				{      
                        if (g_save_audio)
                        {
                            if (paudio_file==NULL)
                            {
                                paudio_file = fopen("pico_audio","wb+");
                            }
                            
                            fwrite(pData, sizeof(char), numFramesAvailable * 4, paudio_file);                                                       
                            
                        }
						AudioSender::GetInstance()->SendAudioBuf((char*)pData, numFramesAvailable * 4,Samples);
#ifndef NO_USBBULK

                        if (pico_streaming::UsbBulkModule::GetInstance()->GetEngineStartup())
                        {
                            pico_streaming::UsbBulkModule::GetInstance()->SendAudioFrame((uint8_t*)pData, numFramesAvailable * 4, Samples);
                            // fwrite(pData, sizeof(char), numFramesAvailable * 4, paudio);
                        }
#endif
				}
				
                if (FAILED(pCaptureClient->ReleaseBuffer(numFramesAvailable)))
                {
                    break;
                }

                if (FAILED(pCaptureClient->GetNextPacketSize(&packetLength)))
                {
                    break;
                }
            }
        }
    }
}
int AudioCaptureSession::GetSample() 
{
	IMMDevice* pDevice = NULL;
	GetDefaultAudioEndpoint(pDevice);
	if (!pDevice) {
		return 48000;
	}
	WAVEFORMATEX* pFormat;
	IAudioClient* pAudioClient = NULL;
	IAudioCaptureClient* pCaptureClient = NULL;
	UINT32 bufferFrameCount;
    if (InitializeAudioClient(pDevice, pAudioClient, pFormat, bufferFrameCount))
    {
        mSamples = pFormat->nSamplesPerSec;
    }
    return mSamples;
		//if( GetCaptureClient(pAudioClient, pCaptureClient))
}
//------------------------------------------------------------------
void AudioCaptureSession::ThreadFunc()
//------------------------------------------------------------------
{
    CoInitialize(NULL);

    //HANDLE hTimerTick = CreatePeriodicTimer(1, 10);
    bool bStarted = false;
    IMMDevice *pDevice = NULL;
    GetDefaultAudioEndpoint(pDevice);
    if (!pDevice) {
        return;
    }
    WAVEFORMATEX* pFormat;
    IAudioClient *pAudioClient = NULL;
    IAudioCaptureClient *pCaptureClient = NULL;
    UINT32 bufferFrameCount;
    if (InitializeAudioClient(pDevice, pAudioClient, pFormat, bufferFrameCount))
    {
        mSamples = pFormat->nSamplesPerSec;
        //if( GetCaptureClient(pAudioClient, pCaptureClient))
        if (!FAILED(pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient)))
        {
            if (!FAILED(pAudioClient->Start()))
            {
                bStarted = true;
            }
        }
    }
	HANDLE hTimerTick = CreatePeriodicTimer2();
   
    HANDLE hHandles[] = { hStopThreadEvent, hTimerTick };

    if (bStarted)
    {
        while (true)
        {
            DWORD dwWaitResult = WaitForMultipleObjects(2, hHandles, FALSE, INFINITE);
            if (dwWaitResult == WAIT_OBJECT_0)
            {
                break;
            }
            Process(pCaptureClient,mSamples );
        }
    }

    { // Cleanup
        ::CoTaskMemFree(pFormat);
        RELEASE(pDevice)
        RELEASE(pAudioClient)
        RELEASE(pCaptureClient)
    }
}
