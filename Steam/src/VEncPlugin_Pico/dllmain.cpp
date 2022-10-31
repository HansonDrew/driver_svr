// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "GlobalDLLContext.h"

#define VENCPLUGINAMF_DLL_NAME "VEncPlugin.dll"

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("====PicoVR VEncPlugin implementation process attached====");

		GLOBAL_DLL_CONTEXT()->SetDLLModule(hModule);
		std::string dllName = VENCPLUGINAMF_DLL_NAME;
		GLOBAL_DLL_CONTEXT()->SetDLLName(dllName);
		break;
	}
	case DLL_THREAD_ATTACH:
	{
		break;
	}
	case DLL_THREAD_DETACH:
	{
		break;
	}
	case DLL_PROCESS_DETACH:
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("====PicoVR VEncPlugin implementation process detached====");
		break;
	}
	default: ;
	}
	return TRUE;
}

