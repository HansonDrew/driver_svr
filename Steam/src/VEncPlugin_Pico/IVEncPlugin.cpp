// VEncPlugin_AMF.cpp : 定义 DLL 的导出函数。
//

#include "pch.h"
#include "framework.h"
#include "../RVRPlugin/IVEncPlugin.h"
#include "../RVRUtils/RVRUtils.h"
#include "VEncPluginPico.h"
 
#include "GlobalDLLContext.h"
#include "../RVRPlugin/RVRPluginDefinitions.h"
char gPluginName[] = "VEncPlugin.dll";
int gRgbType = 0;
RVR::RVRPoseHmdData gRenderPoseList[POSELISTSIZE];
uint64_t gRenderPoseIndex = 0;//store in a list ，left and right get by themselves index （leftindex and rightindex）
 
bool leftgDstFlag = false;
bool rightgDstFlag = false;
std::string gDstIp;
LogFun gLogFun = nullptr;
PushEncodedFrameFun gPushEncodedFrameFun = nullptr;
extern "C" {
	VENCPLUGIN_API RVR::IVEncPlugin* CreateVEncPlugin()
	{
#ifdef picoModel
		VEncPluginPico* instance = new VEncPluginPico();
		//VEncPluginPicoNullDllForQualcomm* instance = new VEncPluginPicoNullDllForQualcomm(); 
#else
		VEncPluginPico* instance = new VEncPluginPico();
#endif
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("CreateVEncPlugin");
		return instance;			
	}
	
	VENCPLUGIN_API UINT32  RVR::GetIVEncPluginVersion()
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("GetIVEncPluginVersion");
		return IVENCPLUGIN_VER;
	}

	VENCPLUGIN_API  CHAR* RVR::GetVEncPluginName()
	{
		return gPluginName;
	}
	VENCPLUGIN_API RVR::IVEncPlugin* CreateVEncPluginPico()
	{
#ifdef picoModel
		gRgbType = 1;//pico 编码 rgb-> h265 VEncPluginPico启用时同步启用。高通是nv12-> h264
		VEncPluginPico* instance = new VEncPluginPico();

#else
		VEncPluginPicoNullDllForQualcomm* instance = new VEncPluginPicoNullDllForQualcomm();
#endif				
		return instance;

	}
	VENCPLUGIN_API void RVR::SetPoseCache(void* pose)
	{
		memmove(&gRenderPoseList[gRenderPoseIndex % POSELISTSIZE], pose, sizeof(RVR::RVRPoseHmdData));
		gRenderPoseList[gRenderPoseIndex % POSELISTSIZE].poseTimeStamp = gRenderPoseIndex;
		gRenderPoseIndex++;
		return;
	}
	VENCPLUGIN_API void RVR::SetPoseDepth(float depth)
	{
		int index = gRenderPoseIndex % POSELISTSIZE;
		index = index > 0 ? index - 1 : POSELISTSIZE - 1;
		gRenderPoseList[index].predictedTimeMs = depth;
		if ((0x8000 & GetAsyncKeyState('D')) != 0 && (0x8000 & GetAsyncKeyState('F')) != 0)
		{
			WCHAR strBuffer[256];
			swprintf_s(strBuffer, 256, L"send out depth = % f\n", gRenderPoseList[index].predictedTimeMs);
			OutputDebugStringW(strBuffer);
			std::string msg = "depth=" + std::to_string(gRenderPoseList[index].predictedTimeMs);
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
		}
		/*	std::string msg = "depth" + std::to_string(gRenderPoseList[index].poseRecvTime)+"index="+std::to_string(index)+"g_index"+std::to_string(gRenderPoseIndex % POSELISTSIZE);
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);*/
		return;
	}
	VENCPLUGIN_API void RVR::SetIp(char* dstip, int len)
	{

		gDstIp = dstip;
		leftgDstFlag = true;
		rightgDstFlag = true;
		return;
	}
	VENCPLUGIN_API void RVR::RegistPushEncodedFrameFun(PushEncodedFrameFun funptr)
	{
		gPushEncodedFrameFun = funptr;
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("RegistPushEncodedFrameFun");

	}
	VENCPLUGIN_API void RVR::RegistLogFun(LogFun funptr)
	{
		gLogFun = funptr;
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("RegistLogFun");

	}
}
 