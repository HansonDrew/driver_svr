#pragma once
#include <windows.h>
#include <string>

#include "pxrTool/LogTool.h"
#include "GlobalDLLConfig.h"

class GlobalDLLContext
{
public:
	static GlobalDLLContext* GetInstance();

	HMODULE GetDLLModule();
	void SetDLLModule(HMODULE module);

	std::string GetDLLName();
	void SetDLLName(std::string& name);

	std::string GetDLLDirectory();

	GlobalDLLConfig GetGlobalDLLConfig();

	pxrLogTool* GetLogger();
	
	void Wchar_tToString(std::string& szDst, wchar_t* wchar);

	class Gc
	{
	public:
		~Gc()
		{
			delete gInstance;
		}
	};
	
private:
	static GlobalDLLContext* gInstance;
	static Gc gGc;
	GlobalDLLContext();
	~GlobalDLLContext();

	HMODULE dllModule;
	std::string dllName;
	pxrLogTool* log;
};

#define GLOBAL_DLL_CONTEXT() GlobalDLLContext::GetInstance()
#define GLOBAL_DLL_CONTEXT_LOG() GlobalDLLContext::GetInstance()->GetLogger()