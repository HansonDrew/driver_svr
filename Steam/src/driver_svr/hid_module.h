#ifndef HIDMODULE_H_
#define HIDMODULE_H_ "HidModule.h"
#include "hidapi.h"
#include <functional>
#include <windows.h> 
#include "driver_define.h"

class HidModule
{

protected:
	HidModule();
	~HidModule();
public:

	static unsigned int __stdcall ReadHmiDataThread(LPVOID lpParameter);
	HANDLE  read_thread_event_=INVALID_HANDLE_VALUE; 
	int64_t last_hmd_recv_timestamp = -1;
	static unsigned int __stdcall CheckHmiDataThread(LPVOID lpParameter);
	HANDLE  check_thread_event_ = INVALID_HANDLE_VALUE;
	void StartUp();
	void CleanUp();
	
	class GC // 垃圾回收类
	{
	public:
		GC()
		{
		}
		~GC()
		{
			// We can destory all the resouce here, eg:db connector, file handle and so on

			if (HidModuleInstance != NULL)
			{
				delete HidModuleInstance;
				HidModuleInstance = NULL;
			}


		}
	};
	static GC HidModuleGc;  //垃圾回收类的静态成员
	int InitHidDevice();

	int OpenHidDevice(const unsigned short vid, const unsigned short pid, const wchar_t *serial_number_ = NULL);

	int ReadData(unsigned char* buf, size_t size = 64);

	int WriteData(unsigned char* buf, size_t size = 64);

	void SetCallbackFunction(std::function<void()> exec);
	int CloseHidDevice();
	static HidModule* GetInstance();
	bool GetLoop_() { return loop_; };
	void SetLoop_(bool loop_t) { loop_ = loop_t; };

	void LogOutHidBuf(unsigned char *buf);
	void  WorkLoop();
	int OperateHidDate(HidType::HidCommon hidcommon);
	int OperateHmdData(HidType::HidCommon hidcommon);
	int OperateControllerData(HidType::HidCommon hidcommon);
	int OperateIpd(HidType::HidCommon hidcommon);
	int OperateDistortionOffser(HidType::HidCommon hidcommon);
	static unsigned int __stdcall ChaperOneSetThread(LPVOID lpParameter);
	unsigned int GetChaperOneSetThreadId_() { return chaper_one_threadid_; };
private:
	unsigned int chaper_one_threadid_;
	bool loop_ = false;
	static HidModule* HidModuleInstance;
	hid_device* m_Hidhandle;
	CRITICAL_SECTION  m_mutex;
	std::function<void()> m_CallBackExec;
};


#endif // !HIDMODULE_H_

