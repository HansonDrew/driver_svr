#include "PluginManger.h"
#include "driverlog.h"
#include "config_reader.h"
#ifndef NO_RTC
#include "ByteRtcMoudel.h"
#endif

#ifndef NO_USBBULK
#include "UsbBulkModule.h"
#endif

#include "TimeTool.h"
#ifndef NO_SLARDAR
#include "SlardarMoudle.h"
#endif  
extern ConfigReader gConfigReader;
int LogOut(const char* buf, int buflen) 
{
	string log = buf;
#ifndef NO_SLARDAR
	if (log.find("slardar")!=std::string::npos)
	{
		char message[4096] = {0};
		memmove(message, buf+ strlen("slardar"), buflen - strlen("slardar"));
		SlardarMoudle::GetInstance()->SaveMessage(message);
	}
	
#endif  
	return 0;
}

int PushEncodedFrame(char* buf, int buflen, int eye_index, EncodeOutWithInfo encode_info)
{
	if (gConfigReader.GetRtcOrBulkMode_() == 1)
	{

#ifndef NO_RTC

		char itype = 0;
		int picture_type = 0;
		if (gConfigReader.GetHEVC() == 0)
		{
			itype = ((buf[4]) & 0x1f);
			if (itype == 5 || itype == 7)
			{
				picture_type = bytertc::VideoPictureType::kVideoPictureTypeI;
			}
			else
			{
				picture_type = bytertc::VideoPictureType::kVideoPictureTypeP;
			}
		}
		else
		{
			itype = ((buf[4]) & 0x7E) >> 1;
			if (itype == 32)
			{
				picture_type = bytertc::VideoPictureType::kVideoPictureTypeI;
			}
			else if ((itype >= 16) && (itype <= 21))
			{
				picture_type = bytertc::VideoPictureType::kVideoPictureTypeI;
			}
			else
			{
				picture_type = bytertc::VideoPictureType::kVideoPictureTypeP;
			}
		}
		int64_t timestamp = GetTimestampUs();
		pico_streaming::ByteRtcMoudel::GetInstance()->SendVideoFrame((uint8_t*)buf, buflen, picture_type, eye_index, timestamp);
#endif
	}
	else
	{
#ifndef NO_USBBULK

		int64_t timestamp = GetTimestampUs();
		//pico_streaming::ByteRtcMoudel::GetInstance()->SendVideoFrame((uint8_t*)buf, buflen, picture_type, eye_index, timestamp);
		int picture_type = 0;
		pico_streaming::UsbBulkModule::GetInstance()->SendVideoFrame((uint8_t*)buf, buflen, picture_type, eye_index, timestamp,encode_info);
#endif

	}
	return 1;
}
bool PluginManger::LoadPlugin(string plugin)
{
	mVEncPlugin = LoadLibraryA(plugin.c_str());
	if (mVEncPlugin == NULL)
	{
		return false;
	}
	return true;
}

bool PluginManger::GetFunPtrOfSetPoseCache()
{	 
	mSetPoseCachePtr = (SetPoseCacheFunPtr)GetProcAddress(mVEncPlugin, "SetPoseCache");
	if (mSetPoseCachePtr != NULL)
	{
		return true;
	}
	return false;	
}

bool PluginManger::GetFunPtrOfCreateVEncPlugin()
{
	mGreateEnvcFunPtr = (GreateEnvcFunPtr)GetProcAddress(mVEncPlugin, "CreateVEncPluginPico");
	if (mSetPoseCachePtr != NULL)
	{
		DriverLog("GreateEnvcFunPtr success");
		return true;
	}
	return false;
}

bool PluginManger::GetFunPtrOfSetDstIpPlugin()
{
	mSetDstIpFunPtr= (SetDstIpFunPtr)GetProcAddress(mVEncPlugin, "SetIp");
	if (mSetDstIpFunPtr != NULL)
	{
		DriverLog("mSetDstIpFunPtr success");
		return true;
	}
	return false;
}

bool PluginManger::GetFunPtrOfSetPoseDepthPlugin()
{
	mSetPoseDepthPtr = (SetPoseDepthPtr)GetProcAddress(mVEncPlugin, "SetPoseDepth");
	if (mSetPoseDepthPtr != NULL)
	{
		DriverLog("SetPoseDepth success");
		return true;
	}
	return false;
}

bool PluginManger::GetFunPtrOfRegistLogFun()
{
	mRegistLogFunPtr = (RegistLogFun)GetProcAddress(mVEncPlugin, "RegistLogFun");
	if (mRegistLogFunPtr != NULL)
	{
		mRegistLogFunPtr(LogOut);
		DriverLog("mRegistLogFunPtr success");
		return true;
	}
	DriverLog("mRegistLogFunPtr failed");

	return false;

}


bool PluginManger::GetFunPtrOfRegistPushEncodedFrame()
{
	mRegistPushEncodedFramePtr = (RegistPushEncodedFrame)GetProcAddress(mVEncPlugin, "RegistPushEncodedFrameFun");
	if (mRegistPushEncodedFramePtr != NULL)
	{
		mRegistPushEncodedFramePtr(PushEncodedFrame);
		DriverLog("mRegistPushEncodedFramePtr success");
		return true;
	}
	DriverLog("mRegistPushEncodedFramePtr failed");

	return false;

}

void PluginManger::DoSetPoseCache(void * pose)
{
	mSetPoseCachePtr(pose);
}
void PluginManger::DoSetPoseDepth(float depth)
{
	mSetPoseDepthPtr(depth);
}
void PluginManger::SetDstIp(string dstip)
{
	mSetDstIpFunPtr((char*)dstip.c_str(),dstip.length());
}
