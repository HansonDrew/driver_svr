#include "base_hmd.h"
#include"driverlog.h"

bool BaseHmd::OnActive(bool is_dp) 
{
	
	
	bool ret = true;
	is_dp_ = is_dp;
	
	mic_audio_session_ = new MicAudioSession();
	ret=mic_audio_session_->StartUp(is_dp);
	
	return ret;
}

bool BaseHmd::OnClearUp() 
{
	DriverLog("base hmd onclearup");
	if (mic_audio_session_)
	{
		DriverLog("pico_mic_log mic shutdown");
		mic_audio_session_->ShutDown();
		mic_audio_session_ = nullptr;

	}
	
	return true;
}

void BaseHmd::SaveMicDate(char* buf, int len)
{
	if (mic_audio_session_)
	{
		mic_audio_session_->SaveBuf(buf, len);
	}
}