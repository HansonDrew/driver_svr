#pragma once
#include <string>
#include <windows.h>
#include "../RVRPlugin/IVEncPlugin.h"
using namespace std;
int LogOut(const char* buf, int buflen);
int PushEncodedFrame(char* buf, int buflen, int eye_index, EncodeOutWithInfo encode_info);
class PluginManger
{
public:
	bool LoadPlugin(string plugin);
	typedef void(*SetPoseCacheFunPtr)(void* pose);
	typedef void(*SetDstIpFunPtr)(char *dstip, int len);
	typedef void(*SetPoseDepthPtr)(float depth);
	typedef RVR::IVEncPlugin* (*GreateEnvcFunPtr)();
	typedef void(*RegistLogFun)(LogFun);
	typedef void(*RegistPushEncodedFrame)(PushEncodedFrameFun);
	bool GetFunPtrOfSetPoseCache();
	bool GetFunPtrOfCreateVEncPlugin();
	bool GetFunPtrOfSetDstIpPlugin();
	bool GetFunPtrOfSetPoseDepthPlugin();
	bool GetFunPtrOfRegistLogFun();
	bool GetFunPtrOfRegistPushEncodedFrame();
	HINSTANCE mVEncPlugin;
	void DoSetPoseCache(void *pose); 
	void DoSetPoseDepth(float depth);
	SetPoseCacheFunPtr mSetPoseCachePtr=NULL;
	GreateEnvcFunPtr mGreateEnvcFunPtr;
	SetDstIpFunPtr mSetDstIpFunPtr;
	SetPoseDepthPtr mSetPoseDepthPtr;
	void SetDstIp(string dstip);
	RegistLogFun mRegistLogFunPtr;
	RegistPushEncodedFrame mRegistPushEncodedFramePtr;
};

