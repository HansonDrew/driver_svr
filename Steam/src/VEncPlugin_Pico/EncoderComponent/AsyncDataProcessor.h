#pragma once

#include <windows.h>
#include <cstdio>
#include <functional>
#include <process.h>

typedef std::function<void(const void* data, size_t len, void* userData)> AdpProc;


#define DEFAULT_CACHE_BUFFER_NUMBER 10
#define DEFAULT_CACHE_BUFFER_LENGTH 4 * 1024 * 1024

struct AdpCache
{
	unsigned char* buf;
	int length;
};


class AsyncDataProcessor
{

public:
	AsyncDataProcessor();
	~AsyncDataProcessor();

	//
	bool Post(unsigned char* data, int len, void* userData = nullptr);

	//
	void Start();

	//
	void Stop();

	//
	void SetProcessFunction(AdpProc proc);


private:

	//
	HANDLE GetStartEvent();

	//
	HANDLE GetWorkThread();

	//
	unsigned int GetWorkThreadID();

	//
	bool GetIsStopping();

	//
	unsigned int GetNewFrameMessageNumber();

	//
	unsigned int GetQuitMessageNumber();


	//
	AdpProc GetProcessFunc();

	//
	AdpCache* GetCacheBuffer(int index);
	
	//
	static unsigned __stdcall WorkProc(void* Param);

	//
	bool quit = false;

	//
	AdpProc cacheProcess = nullptr;

	//Thread start event
	HANDLE startEvent = nullptr;
	

	//
	HANDLE workThread = nullptr;

	//
	unsigned int workThreadID = 0;

	//
	int newFrameMessageNumber = 0;
	
	//
	int quitMessageNumber = 0;

	//
	unsigned int frameIndex = 0;

	//
	AdpCache cacheBuffers[DEFAULT_CACHE_BUFFER_NUMBER] = { {nullptr, 0} };
	
	//
	static unsigned int instanceNumber;

	
};
