#pragma once

#include<WinSock2.h>
#define AUDIOBUFSIZE     200
#define BASEBUFLEN 4096
typedef struct _TcpAudioBuf
{
	char *buf;
	int buf_len;
	int data_len;
}TcpAudioBuf;
class AudioTcp
{
public:
	AudioTcp();
	~AudioTcp() {};
	void InitAudioTcp(u_short port,int delay_buf);
	u_short locate_port_ = 0;
	HANDLE recv_thread_event_ = INVALID_HANDLE_VALUE;
	static unsigned int __stdcall RecvThread(LPVOID parameter);
	bool mLoop;
	void SendAudioBuf(char* buf, int len, int samples);
	bool GetLoop() { return loop_; };
	bool loop_ = false;
	TcpAudioBuf audio_buf_[AUDIOBUFSIZE];
	
	static AudioTcp* GetInstance();
	int buf_index_;
	int send_index_;
	int delay_buffer_;
	char* send_buf_;
	int send_buf_len_ = 4096;
	SOCKET locate_socket_;
	SOCKET hmd_socket_;
	SOCKADDR_IN hmd_addr_;
	class GC
	{
	public:
		GC()
		{
		}
		~GC()
		{
			if (instance_ != NULL)
			{
				delete instance_;
				instance_ = NULL;
			}
		}
	};
	static GC gc;  

	int CloseSocket();
	int CloseHmdSocket();
	 
#define  AudioBufLen  1024*1024
	char recv_buf_[AudioBufLen];
	int recv_len_ = 0;
private:
	static AudioTcp* instance_;
};
