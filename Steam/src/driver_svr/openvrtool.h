//file add by dzhuang
#pragma once

#include <string>
#include<vector>
//#include <windows.h>
//#include <atlbase.h>

using namespace std;

#define BUFSIZE 4096
#define TARGETSTRING "Streaming Assistant"//"RVRRenderer"//
#define TARGETSTRINGW L"Streaming Assistant"
// find the path of vrpathreg.exe
string GetVRPathRegPath();

//get the dirver witch is provided by TARGETSTRING(PicoVR Streaming Assistant),The driver's path is written in a file named openvrpaths.vrpath
bool GetDriverPathFromJson(TCHAR *szPath, TCHAR*szCmd, string &driverpath);
//get the dirver witch is provided by TARGETSTRING(PicoVR Streaming Assistant),The driver's path is written in a file named openvrpaths.vrpath
bool GetRenderModelPathFromJson(TCHAR *szPath, TCHAR*szCmd, string &driverpath);
//make command , call GetDriverPathFromJson to get the dirver witch is provided by TARGETSTRING(PicoVR Streaming Assistant)
bool GetDriverPath(string& driverpath);

bool ChangeRenderModelFile(int type);

bool GetExterDriverList(vector<string>&driverList, vector<wstring>&wdriverList);