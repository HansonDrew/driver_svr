#include "config_reader.h"
#include <string>
#include <windows.h>
#include "stringtool.h"

using namespace std;
extern int gConfigRateChange ;
ConfigReader::ConfigReader() 
{ 
}

void ConfigReader::ReadConfig(string configPath)
{
	wstring wdriverPath = String2WString(configPath);
	mWconfigPath = wdriverPath;
	mAverageBitRate = GetPrivateProfileInt(L"codec", L"bitrate", 20000000, wdriverPath.c_str());
	mMaxBitRate = GetPrivateProfileInt(L"codec", L"maxBitrate", 30000000, wdriverPath.c_str());
	if (mAverageBitRate<1000000 )
	{
		mAverageBitRate = 1000000;
		mMaxBitRate = 1200000;
	}
	if (mAverageBitRate >120000000)
	{
		mAverageBitRate = 120000000;
		mMaxBitRate = 140000000;
	}
	if (mMaxBitRate<=mAverageBitRate)
	{
		mMaxBitRate = mAverageBitRate + mAverageBitRate / 5;
	}
	mGopSize = GetPrivateProfileInt(L"PICO", L"codegop", 4, wdriverPath.c_str());
	mRateControllModel = GetPrivateProfileInt(L"PICO", L"coderatecontrol",1, wdriverPath.c_str());
	mFixedFoveatedEncode = GetPrivateProfileInt(L"PICO", L"FixedFoveatedEncode", 0, wdriverPath.c_str());
	mIfHEVC= GetPrivateProfileInt(L"PICO", L"HEVC", 1, wdriverPath.c_str());
	mIfSlices= GetPrivateProfileInt(L"PICO", L"SLICES", 0, wdriverPath.c_str());
	mCutx = GetPrivateProfileInt(L"PICTURE", L"cutx", 448, wdriverPath.c_str());
	mCuty = GetPrivateProfileInt(L"PICTURE", L"cuty", 448, wdriverPath.c_str());
	crc_= GetPrivateProfileInt(L"PICO", L"crc", 0, wdriverPath.c_str());
	mBigPicture = GetPrivateProfileInt(L"PICO", L"bigpicture", 1, wdriverPath.c_str());
	wchar_t configMsg[256] = { 0 }; 
	GetPrivateProfileString(L"NET", L"dstip", L"192.168.50.148", configMsg, 256, wdriverPath.c_str());
	mdstip = WString2String(configMsg);
	mWidth = GetPrivateProfileInt(L"session", L"eyeWidth", 1920, wdriverPath.c_str());
	mHeight = GetPrivateProfileInt(L"session", L"eyeHeight", 1920, wdriverPath.c_str());
	fps = GetPrivateProfileInt(L"session", L"fps", 36, wdriverPath.c_str());
	mRtpSlice= GetPrivateProfileInt(L"PICO", L"RTPSLICES", 0, wdriverPath.c_str());
	
	mTcp= GetPrivateProfileInt(L"PICO", L"tcp", 1, wdriverPath.c_str());
	mPortL = GetPrivateProfileInt(L"PICO", L"portl", 29702, wdriverPath.c_str());
	mPortR = GetPrivateProfileInt(L"PICO", L"portr", 29704, wdriverPath.c_str());
	mTcpChannel = GetPrivateProfileInt(L"PICO", L"tcpchannel", 1, wdriverPath.c_str()); 
	mOutFile= GetPrivateProfileInt(L"Function", L"outfile", 0, wdriverPath.c_str());
	rtc_mode_= GetPrivateProfileInt(L"Function", L"rtcmode", 0, wdriverPath.c_str());
	mLog = GetPrivateProfileInt(L"PICO", L"log", 0, wdriverPath.c_str());
	mTcp = 1;
	mBigPicture = 1;
}

int ConfigReader::GetRtcOrBulkModeFromFile_()
{
	rtc_mode_ = GetPrivateProfileInt(L"Function", L"rtcmode", 0, mWconfigPath.c_str());
	return rtc_mode_;
}
int  ConfigReader::GetFrameControlType() 
{
	mFrameControlType= GetPrivateProfileInt(L"PICO", L"framecontrol", 0, mWconfigPath.c_str());
	return mFrameControlType;
}
int  ConfigReader::GetAutoRate()
{
	mAutoRate = GetPrivateProfileInt(L"PICO", L"autorate", 1, mWconfigPath.c_str());
	return mAutoRate;
}
int ConfigReader::SetGopSize(int gop_size)
{
	if (gop_size!=0)
	{
		mGopSize = gop_size;
		
	}
	else
	{
		if (mGopSize == 72)
		{
			mGopSize = 36;
		}
		else if (mGopSize == 36)
		{
			mGopSize = 24;
		}
		else if (mGopSize == 24)
		{
			mGopSize = 18;
		}
		else if (mGopSize == 18)
		{
			mGopSize = 12;
		}
		else if (mGopSize == 12)
		{
			mGopSize = 9;
		}
		else if (mGopSize == 9)
		{
			mGopSize = 8;
		}
		else if (mGopSize == 8)
		{
			mGopSize = 6;
		}
		else if (mGopSize == 6)
		{
			mGopSize = 4;
		}
		else if (mGopSize == 4)
		{
			mGopSize = 3;
		}
		
	}
	return mGopSize;
}
ConfigReader::~ConfigReader()
{
}

int ConfigReader::GetFec()
{
	mFecSwitch = GetPrivateProfileInt(L"PICO", L"fec", 0, mWconfigPath.c_str());
	return mFecSwitch;
}
int ConfigReader::GetRtt()
{
	mRttSwitch = GetPrivateProfileInt(L"PICO", L"rtt", 0, mWconfigPath.c_str());
	return mRttSwitch;
}
int ConfigReader::GetRtpAck()
{
	mRtpAck = GetPrivateProfileInt(L"PICO", L"rtpack", 0, mWconfigPath.c_str());
	return mRtpAck;
}

int ConfigReader::GetBitRate()
{
	int old_bit_rate = mAverageBitRate;
	mAverageBitRate = GetPrivateProfileInt(L"codec", L"bitrate", 20000000, mWconfigPath.c_str());
	mMaxBitRate = GetPrivateProfileInt(L"codec", L"maxBitrate", 30000000, mWconfigPath.c_str());
	if (old_bit_rate!=mAverageBitRate)
	{
		gConfigRateChange = 2;
	}
	if (mAverageBitRate < 1000000)
	{
		mAverageBitRate = 1000000;
		mMaxBitRate = 1200000;
	}
	if (mAverageBitRate > 120000000)
	{
		mAverageBitRate = 120000000;
		mMaxBitRate = 140000000;
	}
	if (mMaxBitRate <= mAverageBitRate)
	{
		mMaxBitRate = mAverageBitRate + mAverageBitRate / 5;
	}
	return mAverageBitRate;
}

int ConfigReader::GetTcp()
{
	mTcp = GetPrivateProfileInt(L"PICO", L"tcp", 0, mWconfigPath.c_str());
	mTcp = 1;
	return mTcp;
}
