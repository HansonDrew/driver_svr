
#include <string>
#include <windows.h>
#include <atlbase.h>
#include "openvrtool.h"
#include "stringtool.h"
#include "filetool.h"
using namespace std;


// find the path of vrpathreg.exe
string GetVRPathRegPath()
{
	HKEY hKEY;
	LPCTSTR data_Set = _T("vrmonitor\\Shell\\Open\\Command");
	wchar_t dwValue[1024];
	if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_CLASSES_ROOT, data_Set, 0, KEY_READ, &hKEY))
	{
		DWORD dwSzType = REG_SZ;
		DWORD dwSize = 1024;
		if (::RegQueryValueEx(hKEY, _T(""), NULL, &dwSzType, (LPBYTE)dwValue, &dwSize) != ERROR_SUCCESS)
		{
			printf("error ");
		}
		::RegCloseKey(hKEY);
	}
	char cValue[2048];
	wcharTochar(dwValue, cValue, 2048);
	string retstr = cValue;
	return retstr;
}

//get the dirver witch is provided by TARGETSTRING(PicoVR Streaming Assistant),The driver's path is written in a file named openvrpaths.vrpath
bool GetDriverPathFromJson(TCHAR *szPath, TCHAR*szCmd, string &driverpath)
{
	BOOL bRet = FALSE;
	DWORD dwRead = 0;
	DWORD dwAvail = 0;
	char cbBuf[4096] = { 0 };
	HANDLE hReadPipe = NULL;
	HANDLE hWritePipe = NULL;
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	CreatePipe(&hReadPipe, &hWritePipe, &sa, 0);
	STARTUPINFO si = { 0 };
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.hStdError = hWritePipe;
	si.hStdOutput = hWritePipe;
	PROCESS_INFORMATION   pi = { 0 };

	if (!CreateProcess(szPath, szCmd, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(hReadPipe);
		CloseHandle(hWritePipe);

		return false;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	std::string strResult;
	do
	{

		if (!PeekNamedPipe(hReadPipe, NULL, NULL, &dwRead, &dwAvail, NULL) || dwAvail <= 0)
		{
			return false;
		}
		if (ReadFile(hReadPipe, cbBuf, BUFSIZE, &dwRead, NULL))
		{
			if (dwRead == 0)
				break;
			if (dwRead > 0)
			{
				string bufMsg = string(cbBuf);
				int firstPos = bufMsg.find("External Drivers:") + strlen("External Drivers:");
				int len = bufMsg.length() - firstPos;
				string driverStr = bufMsg.substr(firstPos, len);
				driverStr = trim(driverStr);
				std::vector<std::string> dest;
				Split(driverStr, "\r\n", dest);
				string pathstr = "";
				for (int i = 0; i < dest.size(); i++)
				{
					pathstr = trim(dest[i]);
					wchar_t debugmsg[255] = { 0 };
					wstring wpath = String2WString(pathstr);
					swprintf_s(debugmsg, L"pathstr=%s\n", wpath.c_str());
					
					OutputDebugString(debugmsg);
					if (pathstr.find(TARGETSTRING) != pathstr.npos)
					{
						pathstr = pathstr + "/bin/win64";
						driverpath = pathstr;
						printf("%s\r\n", pathstr.c_str());
						return true;
					}
				}
				break;
			}
		}
	} while (TRUE);

	CloseHandle(hReadPipe);
	CloseHandle(hWritePipe);
	return false;
}

//get the dirver witch is provided by TARGETSTRING(PicoVR Streaming Assistant),The driver's path is written in a file named openvrpaths.vrpath
bool GetRenderModelPathFromJson(TCHAR *szPath, TCHAR*szCmd, string &driverpath)
{
	BOOL bRet = FALSE;
	DWORD dwRead = 0;
	DWORD dwAvail = 0;
	char cbBuf[4096] = { 0 };
	HANDLE hReadPipe = NULL;
	HANDLE hWritePipe = NULL;
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	CreatePipe(&hReadPipe, &hWritePipe, &sa, 0);
	STARTUPINFO si = { 0 };
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.hStdError = hWritePipe;
	si.hStdOutput = hWritePipe;
	PROCESS_INFORMATION   pi = { 0 };

	if (!CreateProcess(szPath, szCmd, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(hReadPipe);
		CloseHandle(hWritePipe);

		return false;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	std::string strResult;
	do
	{

		if (!PeekNamedPipe(hReadPipe, NULL, NULL, &dwRead, &dwAvail, NULL) || dwAvail <= 0)
		{
			return false;
		}
		if (ReadFile(hReadPipe, cbBuf, BUFSIZE, &dwRead, NULL))
		{
			if (dwRead == 0)
				break;
			if (dwRead > 0)
			{
				string bufMsg = string(cbBuf);
				int firstPos = bufMsg.find("External Drivers:") + strlen("External Drivers:");
				int len = bufMsg.length() - firstPos;
				string driverStr = bufMsg.substr(firstPos, len);
				driverStr = trim(driverStr);
				std::vector<std::string> dest;
				Split(driverStr, "\r\n", dest);
				string pathstr = "";
				for (int i = 0; i < dest.size(); i++)
				{
					pathstr = trim(dest[i]);
					wchar_t debugmsg[255] = { 0 };
					wstring wpath = String2WString(pathstr);
					swprintf_s(debugmsg, L"pathstr=%s\n", wpath.c_str());
					OutputDebugString(debugmsg);
					if (pathstr.find(TARGETSTRING) != pathstr.npos)
					{
						pathstr = pathstr + "/resources/rendermodels/";
						driverpath = pathstr;
						printf("%s\r\n", pathstr.c_str());
						return true;
					}
				}
				break;
			}
		}
	} while (TRUE);

	CloseHandle(hReadPipe);
	CloseHandle(hWritePipe);
	return false;
}

//make command , call GetDriverPathFromJson to get the dirver witch is provided by TARGETSTRING(PicoVR Streaming Assistant)
bool GetDriverPath(string& driverpath)
{
	string vrpathreg = GetVRPathRegPath();
	const char* steamvrpath = "steamapps\\common\\SteamVR\\";
	vrpathreg = vrpathreg.substr(vrpathreg.find_first_not_of("\""), vrpathreg.find(steamvrpath) + strlen(steamvrpath) - 1);
	vrpathreg += "bin\\win32\\vrpathreg.exe";
	TCHAR cmd[] = _T("show");
	wstring wvrpathreg = String2WString(vrpathreg);
	return GetDriverPathFromJson((TCHAR*)wvrpathreg.c_str(), cmd, driverpath);
}


bool ChangeRenderModelFile(int type)
{
	string vrpathreg = GetVRPathRegPath();
	const char* steamvrpath = "steamapps\\common\\SteamVR\\";
	vrpathreg = vrpathreg.substr(vrpathreg.find_first_not_of("\""), vrpathreg.find(steamvrpath) + strlen(steamvrpath) - 1);
	vrpathreg += "bin\\win32\\vrpathreg.exe";
	TCHAR cmd[] = _T("show");
	wstring wvrpathreg = String2WString(vrpathreg);
	string resourcesPaht;
	bool ret = GetRenderModelPathFromJson((TCHAR*)wvrpathreg.c_str(), cmd, resourcesPaht);
	if (ret == false)
	{
		return false;
	}
	string leftRenderModelPath = resourcesPaht + "pico_neo2_leftcontroller/";
	string rightRenderModelPath = resourcesPaht + "pico_neo2_rightcontroller/";
	DeleteFileFunc(leftRenderModelPath + "pico_neo2_leftcontroller.json");
	DeleteFileFunc(rightRenderModelPath + "pico_neo2_rightcontroller.json");
	if (type == 0 || type == 2)
	{
		CopyFilec(leftRenderModelPath + "pico_neo2_leftcontroller_vive.json", leftRenderModelPath + "pico_neo2_leftcontroller.json");
		CopyFilec(rightRenderModelPath + "pico_neo2_rightcontroller_vive.json", rightRenderModelPath + "pico_neo2_rightcontroller.json");
	}
	else if (type == 1)
	{
		CopyFilec(leftRenderModelPath + "pico_neo2_leftcontroller_oculus.json", leftRenderModelPath + "pico_neo2_leftcontroller.json");
		CopyFilec(rightRenderModelPath + "pico_neo2_rightcontroller_oculus.json", rightRenderModelPath + "pico_neo2_rightcontroller.json");
	}

}

bool GetExterDriverList(vector<string>& driverList, vector<wstring>&wdriverList)
{
	string vrpathreg = GetVRPathRegPath();
	const char* steamvrpath = "steamapps\\common\\SteamVR\\";
	vrpathreg = vrpathreg.substr(vrpathreg.find_first_not_of("\""), vrpathreg.find(steamvrpath) + strlen(steamvrpath) - 1);
	vrpathreg += "bin\\win32\\vrpathreg.exe";
	TCHAR cmd[] = _T("show");
	wstring wvrpathreg = String2WString(vrpathreg);
	BOOL bRet = FALSE;
	DWORD dwRead = 0;
	DWORD dwAvail = 0;
	char cbBuf[4096] = { 0 };
	HANDLE hReadPipe = NULL;
	HANDLE hWritePipe = NULL;
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	CreatePipe(&hReadPipe, &hWritePipe, &sa, 0);
	STARTUPINFO si = { 0 };
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.hStdError = hWritePipe;
	si.hStdOutput = hWritePipe;
	PROCESS_INFORMATION   pi = { 0 };

	if (!CreateProcess(wvrpathreg.c_str(), cmd, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(hReadPipe);
		CloseHandle(hWritePipe);

		return false;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	std::string strResult;
	do
	{

		if (!PeekNamedPipe(hReadPipe, NULL, NULL, &dwRead, &dwAvail, NULL) || dwAvail <= 0)
		{
			return false;
		}
		if (ReadFile(hReadPipe, cbBuf, BUFSIZE, &dwRead, NULL))
		{
			if (dwRead == 0)
				break;
			if (dwRead > 0)
			{
				string bufMsg = string(cbBuf);
				int firstPos = bufMsg.find("External Drivers:") + strlen("External Drivers:");
				int len = bufMsg.length() - firstPos;
				string driverStr = bufMsg.substr(firstPos, len);
				driverStr = trim(driverStr);
				std::vector<std::string> dest;
				Split(driverStr, "\r\n", dest);
				string pathstr = "";
				for (int i = 0; i < dest.size(); i++)
				{
					pathstr = trim(dest[i]);
					wchar_t debugmsg[255] = { 0 };
					wstring wpath = String2WString(pathstr);
					swprintf_s(debugmsg, L"pathstr=%s\n", wpath.c_str());
					driverList.push_back(pathstr);
					wdriverList.push_back(wpath);
					OutputDebugString(debugmsg);

				}
				break;
			}
		}
	} while (TRUE);

	CloseHandle(hReadPipe);
	CloseHandle(hWritePipe);
	return false;
}

