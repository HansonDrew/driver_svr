#include "AsyncDataProcessor.h"



#include "../GlobalDLLContext.h"



unsigned int AsyncDataProcessor::instanceNumber = 0;

struct VSMessage
{
	uint8_t dataIndex;
	void* userData;
};

AsyncDataProcessor::AsyncDataProcessor()
{

	instanceNumber += 1;
	newFrameMessageNumber = WM_USER + instanceNumber * 0x10;  //Use independent Windows message number on different instance.
	quitMessageNumber = WM_USER + instanceNumber * 0x20;
}

AsyncDataProcessor::~AsyncDataProcessor()
{
}

bool AsyncDataProcessor::Post(unsigned char* data, int len, void* userData)
{

	bool rt = false;
	
	if (workThreadID == 0 || newFrameMessageNumber == 0 || quitMessageNumber == 0)
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogError("AsyncDataProcessor is not fully created. Post() failed.");
		return rt;
	}

	if (data == nullptr || len < 1)
	{
		return rt;
	}


	frameIndex += 1;

	int bufferIndex = frameIndex % DEFAULT_CACHE_BUFFER_NUMBER;

	if (0 == memmove_s(cacheBuffers[bufferIndex].buf, DEFAULT_CACHE_BUFFER_LENGTH, data, len))
	{
		cacheBuffers[bufferIndex].length = len;
		
		VSMessage* msg = new VSMessage;
		msg->dataIndex = bufferIndex;
		msg->userData = userData;

		if (!PostThreadMessage(workThreadID, newFrameMessageNumber, (WPARAM)msg, 0))//post thread msg
		{
			GLOBAL_DLL_CONTEXT_LOG()->LogError("AsyncDataProcessor post message failed.");
			delete[] msg;
			return rt;
		}

		rt = true;
	}


	return rt;
}

void AsyncDataProcessor::Start()
{

	//Create buffers
	for (int index = 0; index < DEFAULT_CACHE_BUFFER_NUMBER; index++)
	{
		cacheBuffers[index].buf = new unsigned char[DEFAULT_CACHE_BUFFER_LENGTH];
	}


	//Create thread start event
	startEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (startEvent == 0)
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogTrace("create start event failed.");
		return;
	}

	//Start thread
	workThread = (HANDLE)_beginthreadex(NULL, 0, &AsyncDataProcessor::WorkProc, this, 0, &workThreadID);
	if (workThread == 0)
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogTrace("AsyncDataProcessor start thread failed.");
		CloseHandle(startEvent);
		return;
	}

	//Wait thread start event to avoid PostThreadMessage return errno:1444
	::WaitForSingleObject(startEvent, INFINITE);
	CloseHandle(startEvent);
	GLOBAL_DLL_CONTEXT_LOG()->LogTrace("AsyncDataProcessor started successfully.");

}

void AsyncDataProcessor::Stop()
{
	quit = true;
	if (!PostThreadMessage(workThreadID, quitMessageNumber, 0, 0))//post thread msg
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogError("AsyncDataProcessor post message failed.");
	}
}

void AsyncDataProcessor::SetProcessFunction(AdpProc proc)
{
	cacheProcess = proc;
}

HANDLE AsyncDataProcessor::GetStartEvent()
{
	return startEvent;
}

HANDLE AsyncDataProcessor::GetWorkThread()
{
	return workThread;
}

unsigned int AsyncDataProcessor::GetWorkThreadID()
{
	return workThreadID;
}

bool AsyncDataProcessor::GetIsStopping()
{
	return quit;
}

unsigned int AsyncDataProcessor::GetNewFrameMessageNumber()
{
	return newFrameMessageNumber;
}

unsigned AsyncDataProcessor::GetQuitMessageNumber()
{
	return quitMessageNumber;
}

AdpProc AsyncDataProcessor::GetProcessFunc()
{
	return cacheProcess;
}

AdpCache* AsyncDataProcessor::GetCacheBuffer(int index)
{
	return &(cacheBuffers[index]);
}

unsigned AsyncDataProcessor::WorkProc(void* Param)
{
	GLOBAL_DLL_CONTEXT_LOG()->LogTrace("AsyncDataProcessor::WorkProc thread start.");

	AsyncDataProcessor* vs = static_cast<AsyncDataProcessor*>(Param);

	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	if (!SetEvent(vs->GetStartEvent())) //set thread start event 
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogTrace("set start event failed");
		return 1;
	}

	while (!vs->GetIsStopping())
	{
		if (GetMessage(&msg, nullptr, 0, 0)) //get msg from message queue
		{
			if (vs->GetNewFrameMessageNumber() == msg.message)
			{
				VSMessage* pInfo = (VSMessage*)msg.wParam;
				AdpCache* ca = vs->GetCacheBuffer(pInfo->dataIndex);
				AdpProc proc = vs->GetProcessFunc();
				proc((const void*)ca->buf, ca->length, pInfo->userData);
				delete[] pInfo;
			}
			else if (vs->GetQuitMessageNumber() == msg.message)
			{
				break;
			}
		}
		else
		{
			GLOBAL_DLL_CONTEXT_LOG()->LogTrace("AsyncDataProcessor GetMessage failed.");
		}
	};

	GLOBAL_DLL_CONTEXT_LOG()->LogTrace("AsyncDataProcessor::WorkProc thread end.");


	return 0;
}


