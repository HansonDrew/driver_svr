#include <thread>
#include "mic_audio_session.h"
#include "audio_rtp_packet.h"
#include "RVRLogger.h"
#include "config_reader.h"
#include "driverlog.h"
extern ConfigReader gConfigReader;
using namespace RVR;
using namespace pxr_base;
HANDLE receive_out=NULL;
HANDLE paly_out=NULL;
void MicLogCallback::NotifyLogCallback(LogLevel level, const char* msg)
{
	char msg_log[1024] = { 0 };
	memmove(msg_log, "pico_mic_log", strlen("pico_mic_log"));
	memmove(msg_log + strlen("pico_mic_log"), msg, strlen(msg));
	
	RVR_LOG_A(msg_log);
	DriverLog(msg_log);
}
bool MicAudioSession::StartUp(bool is_dp)
{
	bool ret = true;
	device_ = MicrophoneDeviceInterface::Create(&miclog_);
	ret=device_->Start(!is_dp);
	device_->SetVolum(gConfigReader.GetMicVolumeValue());
	if (ret )
	{
		RVR_LOG_A("mic device start up ok");
	}
	else
	{
		RVR_LOG_A("mic device start up failed");
	}
	receive_out=CreateEvent(NULL, FALSE, FALSE, NULL);
	paly_out= CreateEvent(NULL, FALSE, FALSE, NULL);
	if (is_dp==false)
	{
		loop_ = true;
		ret = AudioReceiver::GetInstance()->InitAudioReceiver(29712);
		_beginthreadex(NULL, 0, &ReceiveThread, this, 0, 0);
		_beginthreadex(NULL, 0, &PlayThread, this, 0, 0);
		RVR_LOG_A("init mic socket", ret);
	}
	return ret;
}

bool MicAudioSession::ShutDown() 
{
	loop_ = false;
	AudioReceiver::GetInstance()->ShutDown();
	device_->Stop();
	WaitForSingleObject(receive_out,2000);
	WaitForSingleObject(paly_out,2000);
	
	MicrophoneDeviceInterface::Destroy(device_);
	device_ = nullptr;
	return true;
}

unsigned int __stdcall MicAudioSession::ReceiveThread(void* lpParameter)
{
	MicAudioSession* session = (MicAudioSession*)lpParameter;
#define MicRecvLen 2048
	char recv_buf[MicRecvLen] = {0};
	int recv_len = 0;
	while (session->GetLoop_())
	{
		recv_len=AudioReceiver::GetInstance()->RecvBuf(recv_buf, MicRecvLen);
		if (recv_len<0)
		{
			return 1;
		}
		else if (recv_len==0)
		{
			continue;
		}
		else if (recv_len>12)
		{
			session->SaveBuf(recv_buf + 12, recv_len - 12);

		}
	}
	SetEvent(receive_out);
	return 1;
}

unsigned int __stdcall MicAudioSession::PlayThread(void* lpParameter)
{
	MicAudioSession* session = (MicAudioSession*)lpParameter;
	int64_t last_time=-1;
	while (session->GetLoop_())
	{
		if (last_time>0)
		{
			int64_t locate_time = RVR::nowInNs();
			int64_t space_time = locate_time - last_time;
			int time_ms =10000000- space_time / 1000000;
			last_time = RVR::nowInNs();
			//RVR_LOG_A("mic sleep %lld",time_ms/1000000);
			Sleep(time_ms/1000000);
		}
		else
		{
			last_time = RVR::nowInNs();
		}
		char audio_buf[1920] = { 0 };
		int audio_len=session->GetBuf(audio_buf);
		if (audio_len>0)
		{
			if (gConfigReader.GetMicWorkValue()==1)
			{
				session->device_->Write(audio_buf, audio_len);
			}
			
		}
	}
	SetEvent(paly_out);
	return 1;
}
void MicAudioSession::SaveBuf(char* buf, int len) 
{
	
	MicAudioBuf audio_buf;
	memmove(audio_buf.buf, buf, len);
	audio_buf.buf_len = len;
	bufs_mutex_.lock();
	data_bufs_.push(audio_buf);
	bufs_mutex_.unlock();

}
int MicAudioSession::GetBuf(char* buf) 
{
	int out_len = 0;
	int buf_size = 0;
	bufs_mutex_.lock();
	buf_size = data_bufs_.size();
	bufs_mutex_.unlock();
	if (buf_size<6)
	{
		return -1;
	}
	while (out_len!=1920&&loop_)
	{
		MicAudioBuf audio_buf = { 0 };
		audio_buf = data_bufs_.front();
		if (audio_buf.buf_len!=960)
		{
			RVR_LOG_A("mic audio not right length");
		}
		memmove(buf + out_len, audio_buf.buf, 960);
		out_len += 960;
		DeleteLastBuf();
	}
	return out_len;
}

void MicAudioSession::DeleteLastBuf() 
{
	bufs_mutex_.lock();
	data_bufs_.pop();
	bufs_mutex_.unlock();
}