#include "config_reader.h"
#include <string>
#include <windows.h>
#include "stringtool.h"
#include "driverlog.h"
using namespace std;
 

int  ComputeEncoderWidth(int& origin_width, int press, int& render_width, int& cut_x)
{
	if (origin_width < 512)
	{
		render_width = 512;
		cut_x = 0;
		return render_width;

	}
	cut_x = origin_width / 3-64;
	int i = cut_x;
	for (; i > 0; i--)
	{
		if (i % press != 0)
		{
			continue;
		}
		render_width = origin_width - i * 2 + i / press * 2;
		if (render_width % 32 == 0)
		{
			break;
		}
	}
	if (i > 0)
	{
		cut_x = i;
	}
	else
	{
		i = cut_x;
		int up_width = origin_width / 2;
		for (; i < up_width; i++)
		{
			if (i % press != 0)
			{
				continue;
			}
			render_width = origin_width - i * 2 + i / press * 2;
			if (render_width % 32 == 0)
			{
				break;
			}
		}
		if (i == up_width)
		{
			render_width = -1;
		}
	}
	if (render_width < 0)
	{
		origin_width--;
		render_width = ComputeEncoderWidth(origin_width, press, render_width, cut_x);
	}
	return render_width;
}


int ComputeEncoderHeight(int& origin_height, int press, int& render_height, int& cut_y)
{
	if (origin_height < 512)
	{
		render_height = 512;
		cut_y = 0;
		return render_height;

	}
	cut_y = origin_height / 3-64;
	int i = cut_y;
	for (; i > 0; i--)
	{
		if (i % press != 0)
		{
			continue;
		}
		render_height = origin_height - i * 2 + i / press * 2;
		if (render_height % 32 == 0)
		{
			break;
		}
	}
	if (i > 0)
	{
		cut_y = i;
	}
	else
	{
		i = cut_y;
		int up_width = origin_height / 2;
		for (; i < up_width; i++)
		{
			if (i % press != 0)
			{
				continue;
			}
			render_height = origin_height - i * 2 + i / press * 2;
			if (render_height % 32 == 0)
			{
				break;
			}
		}
		if (i == up_width)
		{
			render_height = -1;
		}
	}
	if (render_height < 0)
	{
		origin_height--;
		render_height = ComputeEncoderHeight(origin_height, press, render_height, cut_y);
	}
	return render_height;
}


ConfigReader::ConfigReader() :mControllerType(0), mTestTrigger(-1),mSuperModel(0)
{
}
int ConfigReader::GetEncodeFps() 
{
	/*if (fps==72)
	{
		return 76;
	}
	else if (fps==90)
	{
		return 94;
	}
	return  fps +4;*/
	return  fps;
}
void ConfigReader::ReadConfig(string configPath)
{	
    wdriverPath = String2WString(configPath);
	mAverageBitRate = GetPrivateProfileInt(L"codec", L"bitrate", 30000000, wdriverPath.c_str());
	mMaxBitRate = GetPrivateProfileInt(L"codec", L"maxBitrate", 50000000, wdriverPath.c_str());
	mControllerType = GetPrivateProfileInt(L"PICO", L"controllertype", 0, wdriverPath.c_str());
	
	mTestTrigger = GetPrivateProfileInt(L"PICO", L"triggertest", 0, wdriverPath.c_str());
	mSuperModel= GetPrivateProfileInt(L"PICO", L"super", 0, wdriverPath.c_str());
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"PICO",L"leftControllerModelNumber",L"pico", configMsg,256,wdriverPath.c_str());
	mleftControllerModelNumber = WString2String(configMsg);
	GetPrivateProfileString(L"PICO", L"rightControllerModelNumber", L"pico", configMsg, 256, wdriverPath.c_str());
	mrightControllerModelNumber = WString2String(configMsg);
	GetPrivateProfileString(L"PICO", L"HmdModelNumber", L"pico", configMsg, 256, wdriverPath.c_str());
	mHmdModelNumber = WString2String(configMsg);
	GetPrivateProfileString(L"PICO", L"ControllerInputProfilePath", L"pico", configMsg, 256, wdriverPath.c_str());
	mControllerInputProfilePath = WString2String(configMsg);
	GetPrivateProfileString(L"PICO",L"HmdInputProfilePath", L"pico", configMsg, 256, wdriverPath.c_str());
	mHmdInputProfilePath = WString2String(configMsg);
	GetPrivateProfileString(L"PICO", L"leftControllerModelName", L"pico", configMsg, 256, wdriverPath.c_str());
	mleftControllerModelName = WString2String(configMsg);
	GetPrivateProfileString(L"PICO", L"rightControllerModelName", L"pico", configMsg, 256, wdriverPath.c_str());
	mrightControllerModelName = WString2String(configMsg);
	GetPrivateProfileString(L"PICO", L"SystemName", L"pico", configMsg, 256, wdriverPath.c_str());
	mSystemName = WString2String(configMsg);
	GetPrivateProfileString(L"PICO", L"ManufacturerName", L"pico", configMsg, 256, wdriverPath.c_str());
	mManufacturerName = WString2String(configMsg);

	GetPrivateProfileString(L"NET", L"dstip", L"192.168.50.148", configMsg, 256, wdriverPath.c_str());
	mdstip = WString2String(configMsg);

	mAudioCapturePort = GetPrivateProfileInt(L"PICO", L"AudioCapturePort", 0, wdriverPath.c_str());
	mMtu = GetPrivateProfileInt(L"rtp", L"mtu", 0, wdriverPath.c_str());
	mPicoAudioModel = GetPrivateProfileInt(L"PICO", L"PicoAudioModel", 1, wdriverPath.c_str());
	mAudioDelayTime = GetPrivateProfileInt(L"PICO", L"AudioDelayTime", 0, wdriverPath.c_str());
	sensor_together_= GetPrivateProfileInt(L"PICO", L"SensorTogether", 1, wdriverPath.c_str());
	
	mPortL = GetPrivateProfileInt(L"PICO", L"portl", 29702, wdriverPath.c_str());
	mPortR = GetPrivateProfileInt(L"PICO", L"portr", 29704, wdriverPath.c_str());
	mPortH = GetPrivateProfileInt(L"PICO", L"porth", 29722, wdriverPath.c_str());
	mAudioDelayTime = GetPrivateProfileInt(L"PICO", L"AudioDelayTime", 0, wdriverPath.c_str());
	mBigPicture= GetPrivateProfileInt(L"PICO", L"bigpicture", 1, wdriverPath.c_str());
	single_encode= GetPrivateProfileInt(L"PICO", L"singleencode", 0, wdriverPath.c_str());
	mReversal = GetPrivateProfileInt(L"PICO", L"reversal", 0, wdriverPath.c_str());
	mTcp = GetPrivateProfileInt(L"PICO", L"tcp", 0, wdriverPath.c_str());
	if (mAudioDelayTime>500)
	{
		mAudioDelayTime = 500;
	}
    smooth_controller_ = GetPrivateProfileInt(L"Function", L"smoothcontroller", 0, wdriverPath.c_str());
	mGopSize = GetPrivateProfileInt(L"PICO", L"codegop", 4, wdriverPath.c_str());
	
	mLog = GetPrivateProfileInt(L"PICO", L"log", 0, wdriverPath.c_str());
	mUsbLog = GetPrivateProfileInt(L"PICO", L"usblog", 0, wdriverPath.c_str());
	mHEVC = GetPrivateProfileInt(L"PICO", L"HEVC", 0, wdriverPath.c_str());
	mCutx= GetPrivateProfileInt(L"PICTURE", L"cutx", 448, wdriverPath.c_str());
	mCuty = GetPrivateProfileInt(L"PICTURE", L"cuty", 448, wdriverPath.c_str());
	mCutFlag= GetPrivateProfileInt(L"PICTURE", L"cut", 1, wdriverPath.c_str());
	mWidth = GetPrivateProfileInt(L"session", L"eyeWidth", 1920, wdriverPath.c_str());
	mHeight = GetPrivateProfileInt(L"session", L"eyeHeight", 1920, wdriverPath.c_str());
	split_= GetPrivateProfileInt(L"PICO", L"split", 0, wdriverPath.c_str());
	mComPress = 2;
	if (mHeight==1920)
	{
		mCutx = mCuty = 448;
	}else if (mHeight==2560)
	{
		//mCutx = mCuty = 640;
		mCutx = mCuty = 624;
		mComPress = 3;
	}
	else if (mHeight == 2160)
	{
		mCutx = mCuty = 624;
	}
	else if (mHeight == 2304)
	{
		mCutx = mCuty = 576;
	}
	else if (mHeight == 1664)
	{
		mCutx = mCuty = 384;
	}
	 
	mLinearResolation = GetPrivateProfileInt(L"PICTURE", L"linearresolution", 0, wdriverPath.c_str());
	AADT_Func = GetPrivateProfileInt(L"Function", L"AADT_Func",0, wdriverPath.c_str());
	if (mLinearResolation==1)
	{
		mComPress = 2;
		mEncoderWidth = ComputeEncoderWidth(mWidth, mComPress, mEncoderWidth, mCutx);
		mEncoderHeight = ComputeEncoderHeight(mHeight, mComPress, mEncoderHeight, mCuty);
		if (mHeight == 1920)
		{
			mCutx = mCuty = 448;
			mEncoderWidth = GetEveWidth() - (GetCutx() - GetCutx() / GetComPress()) * 2;
			mEncoderHeight = GetEveHeight() - (GetCuty() - GetCuty() / GetComPress()) * 2;
		}
		else if (mHeight == 2160)
		{
			mCutx = mCuty = 624;
			mEncoderWidth = GetEveWidth() - (GetCutx() - GetCutx() / GetComPress()) * 2;
			mEncoderHeight = GetEveHeight() - (GetCuty() - GetCuty() / GetComPress()) * 2;
		}
		else if (mHeight == 1664)
		{
			mCutx = mCuty = 384;
			mEncoderWidth = GetEveWidth() - (GetCutx() - GetCutx() / GetComPress()) * 2;
			mEncoderHeight = GetEveHeight() - (GetCuty() - GetCuty() / GetComPress()) * 2;
		}

		if (mBigPicture == 1)
		{
			mEncoderWidth = mEncoderWidth * 2;
		}
		DriverLog("compute out w=%d, h=%d,ew=%d,eh=%d,cw=%d,ch=%d\n", mWidth,mHeight,mEncoderWidth,mEncoderHeight,mCutx,mCuty);
	}
	else
	{
		if (mBigPicture==1)
		{
			mEncoderWidth  =  GetEveWidth() - (GetCutx() -GetCutx() / GetComPress()) * 2;
			mEncoderHeight = GetEveHeight() - (GetCuty() - GetCuty() / GetComPress()) * 2;
			if (AADT_Func == 1)
			{
				if (mHeight == 2160)
				{
					mEncoderWidth = 1440;
					mEncoderHeight = 1440;
				}
				if (mHeight == 1920)
				{
					mEncoderWidth = 1280;
					mEncoderHeight = 1280;
				}
				if (mHeight == 1664)
				{
					mEncoderWidth = 1104;
					mEncoderHeight = 1104;
				}

			}
			mEncoderWidth = mEncoderWidth * 2;
		}
		else
		{
			if (mCutFlag == 0)
			{
				mEncoderWidth = mWidth;
				mEncoderHeight = mHeight;
			}
			else
			{
				mEncoderWidth = GetEveWidth() - (GetCutx() - GetCutx() / GetComPress()) * 2;
				mEncoderHeight = GetEveHeight() - (GetCuty() - GetCuty() / GetComPress()) * 2;
			}
		}
	}
	not_encode_ = GetPrivateProfileInt(L"Function", L"notencode", 0, wdriverPath.c_str());
	find_history_pose_= GetPrivateProfileInt(L"Function", L"historypose", 0, wdriverPath.c_str());
	fps= GetPrivateProfileInt(L"session", L"fps", 0, wdriverPath.c_str());
	GetPrivateProfileString(L"session", L"interPupilDistance", L"1.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	interPupilDistance = atof(datastr.c_str());

	GetPrivateProfileString(L"session", L"fov", L"1.0", configMsg, 256, wdriverPath.c_str());
    datastr = WString2String(configMsg);
	fov = atof(datastr.c_str());
	GetPrivateProfileString(L"Function", L"hmd", L"neo2", configMsg, 256, wdriverPath.c_str());
	rtc_mode_ = GetPrivateProfileInt(L"Function", L"rtcmode", 0, wdriverPath.c_str());
	mHmdType = WString2String(configMsg);
	max_sensor_store_ = GetPrivateProfileInt(L"PICO", L"MaxSensorStore", 3, wdriverPath.c_str());
	if (mHmdType.compare("neo3") == 0)
	{
		fov = 95.0f;
	}
	else if (mHmdType.compare("phoenix") == 0)
	{
		fov = 104.f;
	}
	mBigPicture = 1;
	mTcp = 1;
}
int ConfigReader::GetMaxSensorStore() 
{
	max_sensor_store_lock_.lock();
	max_sensor_store_ = GetPrivateProfileInt(L"PICO", L"MaxSensorStore", 3, wdriverPath.c_str());
	if (max_sensor_store_<1)
	{
		max_sensor_store_ = 1;
	}
	max_sensor_store_lock_.unlock();
	return max_sensor_store_;
}

int ConfigReader::SetMaxSensorStore(int max_sensor_store)
{
	max_sensor_store_lock_.lock();
	max_sensor_store_ = max_sensor_store;
	if (max_sensor_store_ < 1)
	{
		max_sensor_store_ = 1;
	}
	max_sensor_store_lock_.unlock();
	return max_sensor_store_;
}

int ConfigReader::GetInsertIdrEnable()
{
	mGopSize = GetPrivateProfileInt(L"PICO", L"insertidr",0, wdriverPath.c_str());
	return mGopSize;
}
ConfigReader::~ConfigReader()
{
}
int ConfigReader::GetMicWork_() 
{
	mic_work_ = GetPrivateProfileInt(L"PICO", L"MicWork", 0, wdriverPath.c_str());
	return mic_work_;
}
int ConfigReader::GetRtcOrBulkModeFromFile_() 
{
	rtc_mode_ = GetPrivateProfileInt(L"Function", L"rtcmode", 0, wdriverPath.c_str());
	return rtc_mode_;
}
float ConfigReader::GetBright()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"PICTURE", L"bright", L"1.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	bright_= atof(datastr.c_str());
	return 	bright_ ;
	 
}

float ConfigReader::GetSaturation()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"PICTURE", L"saturation", L"1.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	saturation_ = atof(datastr.c_str());
	return 	saturation_;
}
float ConfigReader::GetSharper()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"PICTURE", L"sharper", L"1.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	shaper = atof(datastr.c_str());
	return 	shaper;
}
float ConfigReader::GetContrast()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"PICTURE", L"contrast", L"1.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	contrast_ = atof(datastr.c_str());
	return 	contrast_;
}

float ConfigReader::GetAlpha()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"PICTURE", L"alpha", L"1.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	alpha_ = atof(datastr.c_str());
	return 	alpha_;
}
float ConfigReader::GetGamma()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"PICTURE", L"gamma", L"1.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	gamma_ = atof(datastr.c_str());
	return 	gamma_;
}
int ConfigReader::GetDepthCompute() 
{
	depth_compute_ = GetPrivateProfileInt(L"Function", L"depth", 1, wdriverPath.c_str());
	return depth_compute_;
}
int ConfigReader::GetSmoothController()
{
	smooth_controller_ = GetPrivateProfileInt(L"Function", L"smoothcontroller", 0, wdriverPath.c_str());
	return smooth_controller_;
}
float ConfigReader::GetLeftPitch()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"controller", L"leftpitch", L"0.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	left_pitch_ = atof(datastr.c_str());
	return 	left_pitch_;
}


float ConfigReader::GetLeftYaw()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"controller", L"leftyaw", L"0.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	left_yaw_ = atof(datastr.c_str());
	return 	left_yaw_;
}


float ConfigReader::GetLeftRoll()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"controller", L"leftroll", L"0.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	left_roll_ = atof(datastr.c_str());
	return 	left_roll_;
}

float ConfigReader::GetLeftAddX()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"controller", L"leftx", L"0.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	left_add_x_ = atof(datastr.c_str());
	return 	left_add_x_;
}

float ConfigReader::GetLeftAddY()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"controller", L"lefty", L"0.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	left_add_y_ = atof(datastr.c_str());
	return 	left_add_y_;
}

float ConfigReader::GetLeftAddZ()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"controller", L"leftz", L"0.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	left_add_z_ = atof(datastr.c_str());
	return 	left_add_z_;
}


float ConfigReader::GetRightPitch()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"controller", L"rightpitch", L"0.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	right_pitch_ = atof(datastr.c_str());
	return 	right_pitch_;
}


float ConfigReader::GetRightYaw()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"controller", L"rightyaw", L"0.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	right_yaw_ = atof(datastr.c_str());
	return 	right_yaw_;
}


float ConfigReader::GetRightRoll()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"controller", L"rightroll", L"0.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	right_roll_ = atof(datastr.c_str());
	return 	right_roll_;
}

float ConfigReader::GetRightAddX()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"controller", L"rightx", L"0.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	right_add_x_ = atof(datastr.c_str());
	return 	right_add_x_;
}

float ConfigReader::GetRightAddY()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"controller", L"righty", L"0.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	right_add_y_ = atof(datastr.c_str());
	return 	right_add_y_;
}

float ConfigReader::GetRightAddZ()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"controller", L"rightz", L"0.0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	right_add_z_ = atof(datastr.c_str());
	return 	right_add_z_;
}

int ConfigReader::GetControllerAccFlag() 
{
	wchar_t configMsg[256] = { 0 };
	mControllerAccFlag= GetPrivateProfileInt(L"PICO", L"controlleracc", 1,   wdriverPath.c_str());	 
	return 	mControllerAccFlag;
}
float ConfigReader::GetVibrationtime() 
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"Function", L"vibrationtime", L"0.03", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	vibrationtime = atof(datastr.c_str());
	return 	vibrationtime;
}
int ConfigReader::GetAdjustControllerType_() 
{
	adjust_controller_type_ = GetPrivateProfileInt(L"Function", L"adjustcontrollerpose", 0, wdriverPath.c_str());
	return adjust_controller_type_;
}
float  ConfigReader::GetControllerpose() 
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"Function", L"controllerpose", L"0",configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	float data = atof(datastr.c_str());
	return 	data;
}

float  ConfigReader::GetHmdpose()
{
	wchar_t configMsg[256] = { 0 };
	GetPrivateProfileString(L"Function", L"hmdpose", L"0", configMsg, 256, wdriverPath.c_str());
	string datastr = WString2String(configMsg);
	float data = atof(datastr.c_str());
	return 	data;
}
int ConfigReader::GetTcp() 
{
	mTcp = GetPrivateProfileInt(L"PICO", L"tcp", 0, wdriverPath.c_str());
	mTcp = 1;
	return mTcp;
}

int ConfigReader::SetAppRun_(int value)
{
	app_run_ = value;
	return app_run_;
}

int ConfigReader::GetNotEncode()
{
	not_encode_ = GetPrivateProfileInt(L"Function", L"notencode", 0, wdriverPath.c_str());
	return not_encode_;
}