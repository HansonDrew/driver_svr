#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )// 设置入口地址
#include <windows.h>

#include <stdio.h>
#include <string>
#include <cstdlib>

#include <openvr.h>
#include "stdafx.h"
 
#include <iostream>
#include<fstream>
#include<tlhelp32.h>
#include <iomanip>
#include "log.h"
#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
//using namespace pxr_base;
bool log_out = false;
std::string WString2String(const std::wstring& ws)
{
	size_t len = ws.length() + 1;
	size_t converted = 0;
	char* CStr;
	CStr = (char*)malloc(len * sizeof(char));
	wcstombs_s(&converted, CStr, len, ws.c_str(), _TRUNCATE);
	std::string strResult = CStr;
	free(CStr);
	return strResult;
}

bool  isProgramRunning(string program_name)
{
	bool ret = false;
	HANDLE info_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //拍摄系统中所有进程的快照
	if (info_handle == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot fail!!\n\n");
		return false;
	}

	PROCESSENTRY32W program_info;
	program_info.dwSize = sizeof(PROCESSENTRY32W);  //设置结构体大小
	int bResult = Process32FirstW(info_handle, &program_info); //获取所有进程中第一个进程的信息
	if (!bResult)
	{
		printf("Process32FirstW fail!!\n\n");
		return false;
	}

	while (bResult)
	{
		string pro_name = WString2String(program_info.szExeFile);
		if (pro_name.compare(program_name) == 0)
		{
			ret = true;
			break;
		}
		//获得下一个进程的进程信息
		bResult = Process32Next(info_handle, &program_info);
	}
	CloseHandle(info_handle);//关闭句柄
	return ret;
}
//-----------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
class CMainApplication
{
public:
	CMainApplication(int argc, char* argv[]);
	virtual ~CMainApplication();

	bool BInit();
	void Shutdown();

private:

	vr::IVRSystem* m_pHMD;
	vr::IVRRenderModels* m_pRenderModels;

};

//-----------------------------------------------------------------------------
// Purpose: Outputs a set of optional arguments to debugging output, using
//          the printf format setting specified in fmt*.
//-----------------------------------------------------------------------------

void dprintf(const char* fmt, ...)
{
	va_list args;
	char buffer[2048];

	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);

	 
		printf("%s", buffer);

	OutputDebugStringA(buffer);
}
 

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMainApplication::CMainApplication(int argc, char* argv[])
	: m_pHMD(NULL)
	, m_pRenderModels(NULL)
{

};

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMainApplication::~CMainApplication()
{
	// work is done in Shutdown
	dprintf("Shutdown");
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::BInit()
{

 
	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Other);
 

	if (eError != vr::VRInitError_None)
	{
		 
		return false;
	}

	 

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::Shutdown()
{
	if (m_pHMD)
	{
		vr::VR_Shutdown();
		m_pHMD = NULL;
	}

}
std::wstring String2WString(const std::string& s)
{
	size_t len = s.length() + 1;
	size_t converted = 0;
	wchar_t* WStr;
	WStr = (wchar_t*)malloc(len * sizeof(wchar_t));
	mbstowcs_s(&converted, WStr, len, s.c_str(), _TRUNCATE);
	wstring wstrResult(WStr);
	free(WStr);
	return wstrResult;
}
 


int main(int argc, char* argv[])
{
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	WSAStartup(socketVersion, &wsaData);
	SOCKET client_socket = socket(AF_INET, SOCK_DGRAM, 0);

	if (client_socket == INVALID_SOCKET)
	{
		return -1;
	}
	sockaddr_in dst_addr;
	dst_addr.sin_family = AF_INET;
	dst_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	dst_addr.sin_port = htons(29722);
	sockaddr_in dst2_addr;
	dst2_addr.sin_family = AF_INET;
	dst2_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	dst2_addr.sin_port = htons(29723);
	CMainApplication* pMainApplication = new CMainApplication(argc, argv);
	if (!pMainApplication->BInit())
	{
		pMainApplication->Shutdown();
		return 1;
	}
	CHAR szPath[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szPath, MAX_PATH);
	RtlZeroMemory(strrchr(szPath, ('\\')), strlen(strrchr(szPath, ('\\'))) * sizeof(CHAR));
	strcat(szPath, "\\");
	strcat(szPath, LOG_FILE_NAME);
	remove(szPath);
	uint32_t ret = 0;
	while (isProgramRunning("vrserver.exe"))
	{

		char app_key[1024] = { 0 };
		uint32_t app_count = vr::VRApplications()->GetApplicationCount();
		int  app_run = 0;
		for (int i = 0; i < app_count; i++)
		{

			vr::VRApplications()->GetApplicationKeyByIndex(i, app_key, 1024);
			ret = vr::VRApplications()->GetApplicationProcessId(app_key);
			string app_name_str = app_key;
			printf("%s  app =%d\n", app_key, ret);
			if (log_out)
			{
				string msg = "app check id=" + app_name_str;
				msg = msg + ":" + std::to_string(ret);
				LOG(msg.c_str());
			}
			if (ret != 0)
			{
				if ((app_name_str.compare("openvr.tool.steamvr_environments") == 0) ||
					app_name_str.find("steam.app.") != std::string::npos)
				{
					printf("%s  runapp =%d\n", app_key, ret);
					app_run = 1;
					if (app_run==1)
					{
						string appid = app_name_str.substr(strlen("steam.app."), app_name_str.length() - strlen("steam.app."));
						if (atoi(appid.c_str())== 1593700)
						{
							app_run = 2;
						}
					}
					break;
				}
			}

		}
		if ((0x8000 & GetAsyncKeyState('L')) != 0 && (0x8000 & GetAsyncKeyState('R')) != 0 && (0x8000 & GetAsyncKeyState('A')) != 0)
		{
			log_out = true;
		}
		if (log_out)
		{
			if (app_run == 1)
			{
				string msg = "app run:" + std::to_string(ret);
				LOG(msg.c_str());
			}
			else
			{
				LOG("no app_run");
			}
			LOG("---------------------------------------------------------------------");
		}

		SYSTEMTIME sys;

		GetLocalTime(&sys);

		printf("%4d/%02d/%02d %02d:%02d:%02d.%03d \n", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
		/*wstring config_path = String2WString("./RVRPlugin.ini");
		BOOL ret=	WritePrivateProfileString(L"PICO", L"apprun",std::to_wstring(app_run).c_str(),config_path.c_str());*/

		std::string msg = "apprun:" + to_string(app_run);

		int ret = sendto(client_socket, msg.c_str(), msg.length(), 0, (sockaddr*)&dst_addr, sizeof(dst_addr));
		char app_name[1024] = { 0 };
		vr::EVRApplicationError error_code;
		int ret_app_name = 0;

		char app_value[1024] = { 0 };
		int ret_app_value = 0;
		if (app_run !=0) 
		{
			
			ret_app_name=vr::VRApplications()->GetApplicationPropertyString(app_key, vr::EVRApplicationProperty::VRApplicationProperty_Name_String, app_name,1024, &error_code);
            ret_app_value= vr::VRApplications()->GetApplicationPropertyString(app_key, vr::EVRApplicationProperty::VRApplicationProperty_ImagePath_String, app_value, 1024, &error_code);
			msg = "apprun:key=";
			msg = msg + app_key;
			msg = msg + ":name=";
			char sendbuf[2048] = { 0 };
			int rel_send = msg.length() + ret_app_name + 1;
			memmove(sendbuf, msg.c_str(), msg.length());
			memmove(sendbuf + msg.length(), app_name, ret_app_name);
			ret = sendto(client_socket, msg.c_str(), rel_send, 0, (sockaddr*)&dst2_addr, sizeof(dst2_addr));
		}
	

		Sleep(100);
	}
	

	pMainApplication->Shutdown();
 
	delete pMainApplication;
	 
	return 0;
}
