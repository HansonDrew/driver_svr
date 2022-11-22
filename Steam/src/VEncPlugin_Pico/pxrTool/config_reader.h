#pragma once
#include <string>
using namespace std;
//#define ORIGINFILETEST //wirte left.h265 and  right.h265 into file
//#define DEBUGWIRTEFILE
#define BITRATECHANGE 5000000
#define MAXBITRATE 120000000
#define MINBITRATE 2500000
class ConfigReader
{
public:

	void ReadConfig(string configPath);
	int GetGopSize() const
	{
		return mGopSize;
	}
	int	SetGopSize(int gop_size=0);
	int GetRateControllModel() const
	{
		return mRateControllModel;
	}

	int GetFixedFoveatedEncode() const
	{
		return mFixedFoveatedEncode;
	}
	int GetIfHEVC() const
	{
		return mIfHEVC;
	}
	int GetIfSlices() 
	{
		return mIfSlices;
	}
	int GetCutx() { return mCutx; };
	int GetCuty() { return mCuty; };
	int GetRtpSlice() { return mRtpSlice; };
	int GetEveWidth() { return mWidth; };
	int GetEveHeight() { return mHeight; };
	string GetDstIp() {
		return mdstip;
	};
	ConfigReader();
	~ConfigReader();
	int GetCrc_() { return crc_; };
	int GetBitRate() ;
	 
	int GetAverageBitRateValue () { return mAverageBitRate; };
	int GetMaxBitRateValue() { return mMaxBitRate; };
	int GetPortL() { return mPortL; };
	int GetPortR() { return mPortR; };
	int GetOutFile() { return mOutFile; };//wirte left.h265 and  right.h265 into file
	int BigPicture() { return mBigPicture; };
	int GetTcpValue() { return mTcp; };
	int GetTcp();
	int GetLog() { return mLog; };
	int GetFps() 
	{
		/*	if (fps == 72)
			{
				return 76;
			}
			else if (fps == 90)
			{
				return 94;
			}
			return  fps + 4;*/
		return  fps;
	};
	int GetRtcOrBulkModeFromFile_();
	int GetTcpChannel() { return mTcpChannel; };
	int GetFrameControlType() ;
	int GetAutoRate();
	int GetAutoRateValue() { return mAutoRate; };
	int GetFec();
	int GetRtt();
	int GetRtpAck();

	int GetFecValue() { return mFecSwitch; };
	int GetRttValue() { return mRttSwitch; };
	int GetRtpAckValue() { return mRtpAck; };
	int GetRtcOrBulkMode_() { return rtc_mode_; }
	int GetBitRateControl() { return mBitRateControl; };
	void SetBitRateControl(int value) { mBitRateControl = value; };
	wstring mWconfigPath;
private:
	int mBitRateControl = 0;
	int rtc_mode_ = 0;
	int mFecSwitch = 0;
	int mRttSwitch = 0;
	int mRtpAck = 0;
	int mAutoRate = 0;
	int mLog;
	int mTcp;
	int mBigPicture;
	int mPortL;
	int mPortR;
	int mRtpSlice = 0;
	int mGopSize = -1;
	int mRateControllModel = -1;
	int mFixedFoveatedEncode = -1;
	int mIfHEVC = -1;
	int mIfSlices=0;
	int mCutx;
	int mCuty;
	int crc_;
	int mWidth;
	int mHeight;
	int fps;
	int mAverageBitRate;
	int mMaxBitRate;
	int mOutFile;
	int mTcpChannel;
	/*0 :control by gop with Ipp . 1:INFINITE_GOPLENGTH . 2: all i  3 all idr  */
	int mFrameControlType;
	string mdstip;
};

