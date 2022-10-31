#include "audio_tcp.h"
#include <process.h>
#include "driverlog.h"
#include "mic_audio_session.h"
#include "base_hmd.h"
#include "driver_pico.h"
# define MicAudioLen 960
extern PicoVRDriver g_svrDriver;
extern bool g_save_audio;
AudioTcp* AudioTcp::instance_ = nullptr;
AudioTcp:: AudioTcp() {
	buf_index_ = send_index_ = delay_buffer_ = 0;
	send_buf_len_ = 4096;
	send_buf_ = new char[send_buf_len_];
}
AudioTcp* AudioTcp::GetInstance()
{

	if (nullptr == instance_)
	{
		instance_ = new AudioTcp();
		instance_->buf_index_ = instance_->send_index_ = instance_->delay_buffer_ = 0;
	}

	return instance_;
}


void AudioTcp::InitAudioTcp(u_short port, int delay_buf)
{
	for (int i=0;i<AUDIOBUFSIZE;i++)
	{
		audio_buf_[i].buf_len = BASEBUFLEN;
		audio_buf_[i].data_len = 0;
		audio_buf_[i].buf = new char[BASEBUFLEN];
	}
	delay_buffer_ = delay_buf;
	
	locate_port_ = port;
	recv_thread_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);

	locate_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct linger so_linger;
	so_linger.l_onoff = 0;
	so_linger.l_linger = 0;
	setsockopt(locate_socket_, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof so_linger);

	int i = 1; setsockopt(locate_socket_, IPPROTO_TCP, TCP_NODELAY, (char*)&i, sizeof(i));
	int chOptVal = 1;
	if (setsockopt(locate_socket_, SOL_SOCKET, SO_REUSEADDR, (char*)&chOptVal, sizeof(int)) == SOCKET_ERROR)
	{
		//OutputDebugString(L"¶Ë¿Ú¸´ÓÃÉèÖÃÊ§°Ü");
	}
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	if (SOCKET_ERROR == bind(locate_socket_, (sockaddr*)&_sin, sizeof(_sin)))
	{
		RVR_LOG_A("ERROR£¬audio tcp bind error");
	}
	else {
		RVR_LOG_A("audio tcp bind ok");
	}

	if (SOCKET_ERROR == listen(locate_socket_, 32))
	{
		RVR_LOG_A("ERROR£¬sensor tcp listen error");
	}
	else {
		RVR_LOG_A("audio tcp listen ok");
	}



	loop_ = true;
	HANDLE ret = (HANDLE)_beginthreadex(NULL, 0, &RecvThread, this, 0, NULL);
	SetThreadPriority(ret, THREAD_PRIORITY_TIME_CRITICAL);
	if (ret != NULL)
	{
		CloseHandle(ret);
	}
}
#define PerBufLen 5000
unsigned int  AudioTcp::RecvThread(LPVOID lpParameter)
{
	
	ResetEvent(AudioTcp::GetInstance()->recv_thread_event_);
	while (AudioTcp::GetInstance()->GetLoop())
	{
		int len = sizeof(SOCKADDR);
		DriverLog("sensor keep listen");
		AudioTcp::GetInstance()->hmd_socket_ = accept(AudioTcp::GetInstance()->locate_socket_, (SOCKADDR*)&AudioTcp::GetInstance()->hmd_addr_, &len);
		struct linger so_linger;
	/*	so_linger.l_onoff = 0;
		so_linger.l_linger = 0;
		setsockopt(AudioTcp::GetInstance()->hmd_socket_, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof so_linger);*/
		DriverLog("audio tcp client connected");
		
		int64_t socket_ts = 0;
		int loopfirst = 0;
		while (AudioTcp::GetInstance()->GetLoop())
		{ 
			char recv_buf[PerBufLen];
			int recv_len = recv(AudioTcp::GetInstance()->hmd_socket_, recv_buf, PerBufLen, 0);
			if (recv_len <= 0)
			{
				//AudioTcp::GetInstance()->CloseHmdSocket();
				break;
			}
			bool save = false;
			if (AudioTcp::GetInstance()->recv_len_ + recv_len<AudioBufLen)
			{
				memmove(AudioTcp::GetInstance()->recv_buf_ + AudioTcp::GetInstance()->recv_len_, recv_buf, recv_len);
				AudioTcp::GetInstance()->recv_len_ = AudioTcp::GetInstance()->recv_len_ + recv_len;
				save = true;
			}
			 
		
			while (AudioTcp::GetInstance()->recv_len_ >=( MicAudioLen+9))
			{
				char msg_type = AudioTcp::GetInstance()->recv_buf_[0];
				if (msg_type == 0x13)
				{
					int msg_len = 0;
					memmove(&msg_len, AudioTcp::GetInstance()->recv_buf_ + 1, sizeof(int));
					if (msg_len == MicAudioLen)
					{		
						int samples = 0;
						memmove(&samples, AudioTcp::GetInstance()->recv_buf_ + 1 + 4, sizeof(int));
						//// play 
						 
						//AudioTcp::GetInstance()->GetSensorFromSocket(TcpSensorSocket::GetInstance()->recv_buf_ + 5, TCPSENSORMSGLEN);
						g_svrDriver.GetStreamingHmdDriver()->SaveMicDate(AudioTcp::GetInstance()->recv_buf_ +9, MicAudioLen);
						AudioTcp::GetInstance()->recv_len_ = AudioTcp::GetInstance()->recv_len_ - (MicAudioLen+9);
						if (AudioTcp::GetInstance()->recv_len_ > 0)
						{
							memmove(AudioTcp::GetInstance()->recv_buf_, AudioTcp::GetInstance()->recv_buf_ + MicAudioLen + 9, AudioTcp::GetInstance()->recv_len_);
						}

					}else{
						DriverLog("msg audio length error,length= %d", msg_len);	
						AudioTcp::GetInstance()->recv_len_ = 0;
					//	AudioTcp::GetInstance()->CloseHmdSocket();
					}
				}
				else
				{
					DriverLog("recv audio msg type error");
					AudioTcp::GetInstance()->recv_len_ = 0;
				//	AudioTcp::GetInstance()->CloseHmdSocket();
				}
				 
			}
			if (save==false)
			{
				if (AudioTcp::GetInstance()->recv_len_ + recv_len < AudioBufLen)
				{
					memmove(AudioTcp::GetInstance()->recv_buf_ + AudioTcp::GetInstance()->recv_len_, recv_buf, recv_len);
					AudioTcp::GetInstance()->recv_len_ = AudioTcp::GetInstance()->recv_len_ + recv_len;
					save = true;
				}
			}
		}

	}
	DriverLog("sensor tcp thread quit ok");
	SetEvent(AudioTcp::GetInstance()->recv_thread_event_);
	return 1;
	 
}
FILE* pSendAuido = NULL;
void AudioTcp::SendAudioBuf(char* buf, int len, int samples) 
{
	if ((len+9)>audio_buf_[buf_index_].buf_len)
	{
		delete[] audio_buf_[buf_index_].buf;
		audio_buf_[buf_index_].buf_len = audio_buf_[buf_index_].buf_len * 2;
		audio_buf_[buf_index_].buf = new char[audio_buf_[buf_index_].buf_len];
	}
	memset(audio_buf_[buf_index_].buf, 0, audio_buf_[buf_index_].buf_len);
	audio_buf_[buf_index_].data_len = 0;
	memmove(audio_buf_[buf_index_].buf, buf , len);
	audio_buf_[buf_index_].data_len = len;
	 
	if (abs(buf_index_ - send_index_) >= delay_buffer_)
	{

		int real_send_len = audio_buf_[send_index_].data_len + 9;
		while (real_send_len>send_buf_len_)
		{
			delete[] send_buf_;
			send_buf_len_ = send_buf_len_ * 2;
			send_buf_ = new char[send_buf_len_];
		}

		memset(send_buf_, 0, send_buf_len_);
		send_buf_[0] = 0x12;
		int data_len = audio_buf_[send_index_].data_len ;
		memmove(send_buf_ + 1, &data_len, sizeof(int));
		memmove(send_buf_ + 5, &samples, sizeof(int));
		memmove(send_buf_ + 9, audio_buf_[send_index_].buf, audio_buf_[send_index_].data_len);
		int ret=send(hmd_socket_, send_buf_, real_send_len, 0);
	
		if (g_save_audio)
		{
			if (pSendAuido == NULL)
			{
				pSendAuido = fopen("pico_send_audio.pcm", "wb+");
			}
			if (pSendAuido != NULL)
			{
				fwrite(send_buf_ + 9, sizeof(char), real_send_len - 9, pSendAuido);
			}
		}
		delay_buffer_ = 0;
		send_index_ = (send_index_ + 1) % AUDIOBUFSIZE;
	}
	buf_index_ = (buf_index_ + 1) % AUDIOBUFSIZE;

}
int  AudioTcp::CloseSocket()
{
	loop_ = false;
	DriverLog(" audio tcp CloseSocket ");
	closesocket(hmd_socket_);
	closesocket(locate_socket_);
	 
	WaitForSingleObject(recv_thread_event_, INFINITE);
	DriverLog("sensor tcp CloseSocket ok");
	return 1;
}
int AudioTcp::CloseHmdSocket()
{
	DriverLog(" audio hmdtcp CloseSocket ");
	closesocket(hmd_socket_);
	recv_len_ = 0;
	memset(recv_buf_, 0, sizeof(char) * AudioBufLen);
	return 1;
}