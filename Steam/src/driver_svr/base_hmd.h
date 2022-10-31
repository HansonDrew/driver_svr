#pragma once
#include "mic_audio_session.h"

class BaseHmd
{
public:
	
	bool OnActive(bool is_dp=false);
	bool OnClearUp();
	void SaveMicDate(char *buf,int len);
	uint64_t last_timestamp_ = 0;
	uint64_t timestamp_ = 0;
	uint64_t last_update_timestamp_ns_ = 0;
private:
	
	bool is_dp_ = false;
	MicAudioSession* mic_audio_session_=nullptr;
};

