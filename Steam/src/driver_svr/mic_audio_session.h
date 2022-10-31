#pragma once
 
#include <queue>
#include <mutex>

#include "microphone_device_interface.h"
 
using namespace pxr_base;
class MicLogCallback :public MicrophoneDeviceCallback
{
public:
	MicLogCallback() = default;
	~MicLogCallback() = default;
	virtual void NotifyLogCallback(LogLevel level, const char* msg);
	 
};

struct MicAudioBuf
{
	char buf[1920];
	int buf_len;
};
class MicAudioSession
{
public:
	std::queue<MicAudioBuf> data_bufs_;
	std::mutex bufs_mutex_;
	bool StartUp(bool is_dp=false);
	MicrophoneDeviceInterface* device_;
	MicLogCallback  miclog_;
	 
	static unsigned int __stdcall ReceiveThread(void* lpParameter);
	static unsigned int __stdcall PlayThread(void* lpParameter);
	void SaveBuf(char *buf,int len);
	int GetBuf(char* buf);
	void DeleteLastBuf();
	bool ShutDown();
	bool GetLoop_() { return loop_; };
private:
	bool loop_ = false;
};

