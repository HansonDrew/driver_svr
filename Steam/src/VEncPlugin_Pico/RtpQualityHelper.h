#pragma once

#include "UpdSocket.h"
#include "fec_packet.h"
#define  MAX_SAVE_BUF 1500
#define STORE_BUF_NUM 3
#define RTTNUM 15
enum class DecreaseFlag
{
	low,
    normal,
	high
};

 
struct RtpStoreBuf
{
	u_short begin_step_no;
	u_short buf_size;
	char* buf [1500];
	int buflen[1500];

};
struct RttBuf
{
	uint64_t sequence;
	uint64_t server_ime;
};
class RtpQualityHelper
{
public:
	RtpStoreBuf mLeftRtpStore[STORE_BUF_NUM];
	int mLeftRtpStoreIndex=0;
	RtpStoreBuf mRightRtpStore[STORE_BUF_NUM];
	int mRightRtpStoreIndex = 0;

	void SaveRtpBuf(char* buf,int buflen,int eyeIndex,bool makefec);
	bool GetAndSendLostRtpBuf(u_short lostNo,  int eyeIndex);
	 
	UpdSocket mUdpSocket;
	static RtpQualityHelper* GetInstance();
	int Init() ;
	int SetSocketParam(SOCKET leftSocket, SOCKET rightSocket, std::string dstIp, u_short leftPort, u_short rightPort);
	int SetBitRate(int averageRate,int maxRate);
	//bool ChangeEncodeBitRate(int index,int &averageRate,int &maxRate, bool& itype);
	void GetCurrentRate(int& averageRate, int& maxRate);
	//void IncreaseBitRate();
	//void DecreaseBitRate(float div=2.0f,int flag=0);
	void ForceMaxBitRate(int averageRate, int maxRate);
	uint64_t mDecreaseTimeTrace=0;
	uint64_t mDecreaseSecondTimeTrace = 0;
	uint64_t mIncreaseTimeTrace=0;
	uint64_t mLastServerTime=0;
	uint64_t mLastClientTime=0;
	uint64_t mLastServerRtt=0;

	uint64_t mLastRateIncreaseTime;
	uint64_t mLastRateDecreaseTime;

	uint64_t mRateIncreaseTimeSpace;
	uint64_t mRateDecreaseTimeSpace;
	bool mLeftChangeRate = false;
	bool mRightChangeRate = false;
	int mIdrCount = 0;
	bool mLeftIdr = false;
	bool mRightIdr = false;
	int mDecreaseCount = 0;
	int mDecreaseSecondCount = 0;
	int mIncreaseCount = 0;
	RttBuf mRttBuf[RTTNUM] = { 0 };
	int mRttIndex = 0;
	uint64_t FindServerTime(uint64_t sequence);
private:
	static RtpQualityHelper* gRtpQualityHelperInstance;
	fec_packet* mLeftFec;
	fec_packet* mRightFec;
	SOCKET mLeftRtpSocket;
	SOCKET mRigthRtpSocket;
	u_short mLeftPort;
	u_short mRightPort;
	std::string mDstIp;

	int minAverageRate;
	int maxAverageRate;

	int minMaxRate;
	int maxMaxRate;

	int curAverageRate;
	int curMaxRate;
};

