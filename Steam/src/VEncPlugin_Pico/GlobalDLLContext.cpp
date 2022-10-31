#include "GlobalDLLContext.h"
#include "pxrTool/config_reader.h"

GlobalDLLContext* GlobalDLLContext::gInstance = nullptr;
GlobalDLLContext::Gc GlobalDLLContext::gGc;

GlobalDLLContext* GlobalDLLContext::GetInstance()
{
	if(nullptr == gInstance)
	{
		gInstance = new GlobalDLLContext();
	}

	return gInstance;
}

HMODULE GlobalDLLContext::GetDLLModule()
{
	return dllModule;
}

void GlobalDLLContext::SetDLLModule(HMODULE module)
{
	this->dllModule = module;
}

std::string GlobalDLLContext::GetDLLName()
{
	return dllName;
}

void GlobalDLLContext::SetDLLName(std::string& name)
{
	this->dllName = name;
}

std::string GlobalDLLContext::GetDLLDirectory()
{
	TCHAR szBuffer[MAX_PATH] = { 0 };
	if (dllModule != nullptr)
	{
		GetModuleFileName(dllModule, szBuffer, sizeof(szBuffer) / sizeof(TCHAR) - 1);
	}

	std::string retStr;
	Wchar_tToString(retStr, szBuffer);

	return retStr.substr(0, retStr.rfind("\\"));
	
}

GlobalDLLConfig GlobalDLLContext::GetGlobalDLLConfig()
{

	string configPath = GetDLLDirectory();
	configPath += "/RVRPlugin.ini";

	ConfigReader cr;
	cr.ReadConfig(configPath);

	GlobalDLLConfig config;
	config.SetGOPSize(cr.GetGopSize());
	config.SetFixedFoveatedEncode(cr.GetFixedFoveatedEncode());
	config.SetRateControllMode(cr.GetRateControllModel());
	return config;
}

pxrLogTool* GlobalDLLContext::GetLogger()
{
	if(nullptr == log)
	{
		log = new pxrLogTool();
	}

	return log;
}

void GlobalDLLContext::Wchar_tToString(std::string& szDst, wchar_t* wchar)
{
	wchar_t* wText = wchar;
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);
	char* psText;
	psText = new char[dwNum];
	WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);
	szDst = psText;
	delete[]psText;
}


GlobalDLLContext::GlobalDLLContext()
	:dllModule(nullptr)
	,dllName("")
	,log(nullptr)
{
}

GlobalDLLContext::~GlobalDLLContext()
{
	dllModule = nullptr;
	dllName = "";
	delete log;
}
