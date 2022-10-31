#include "RtpQualityHelper.h"
#include "../../GlobalDLLContext.h"
#include <string>
#include <process.h>
#include "fec_define.h"
#include "../pxrTool/TimeTool.h"
#include "../pxrTool/config_reader.h"
using namespace std;

extern bool gLog;
extern ConfigReader gConfig;
RtpQualityHelper* RtpQualityHelper::gRtpQualityHelperInstance = NULL;

unsigned int __stdcall RtpQualitySendRttThread(LPVOID lpParameter)
{
	return 0;
	char buf[1024] = { 0 };
	int len = 1024;
	uint64_t sequence = 0;
	int64_t startTime = 0;
	int64_t lastTime = GetTimestampUs();
	while (true)
	{
		if (gConfig.GetRttValue()==0)
		{
			Sleep(5000);
			continue;
		}
		TimeSync timesync;
		memmove((void*)&timesync, buf, sizeof(TimeSync));

		timesync.clientTime = GetTimestampUs();

		timesync.mode = 0;
		timesync.sequence = sequence;

		int len = sizeof(TimeSync);
		string msg = "TimeSync  mode 0 clientTime= " + to_string(timesync.clientTime) +
			",Sequence" + to_string(sequence);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
		RtpQualityHelper::GetInstance()->mRttBuf[RtpQualityHelper::GetInstance()->mRttIndex % RTTNUM].sequence = sequence;
		RtpQualityHelper::GetInstance()->mRttBuf[RtpQualityHelper::GetInstance()->mRttIndex % RTTNUM].server_ime = timesync.clientTime;
		RtpQualityHelper::GetInstance()->mRttIndex++;
		RtpQualityHelper::GetInstance()->mUdpSocket.SendData((char*)(&timesync), len);
		sequence++;
		startTime = GetTimestampUs();
		int64_t offset = 0;
		int64_t intervalt_i64 = 500000 - (startTime - lastTime);
		if (intervalt_i64 > 0)
		{
			int64_t sleep_begin = GetTimestampUs();

			int sleep_time = (intervalt_i64 - offset) / 1000;
			Sleep(sleep_time);
			startTime = GetTimestampUs();
			/*string msg = " sleep=" + std::to_string(sleep_time) + "  intervalt_i64= " + std::to_string(intervalt_i64);
			RVR_LOG_A(msg.c_str());*/
		}

		lastTime = startTime;

	}
	return 1;
}

unsigned int __stdcall RtpQualityNetHelperThread(LPVOID lpParameter)
{
	return 0;
	char buf[1024] = { 0 };
	int len = 1024;

	while (true)
	{
		int recvRet = RtpQualityHelper::GetInstance()->mUdpSocket.RecvData(buf, len);

		if (recvRet != sizeof(TimeSync))
		{
			if (gLog)
			{
				string msg = "RecvData  TimeSync  error " + to_string(recvRet);
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
			}

		}
		else
		{
			TimeSync timesync;
			memmove((void*)&timesync, buf, sizeof(TimeSync));
			if (timesync.mode == 0)
			{

				/*timesync.serverTime = GetTimestampUs();
				timesync.mode = 1;
				uint64_t   Sequence = timesync.sequence;

				int len = sizeof(TimeSync);
				string msg = "TimeSync  mode 0 clientTime= " + to_string(timesync.clientTime)+",serverTime="+to_string(timesync.serverTime)+
					",Sequence"+to_string(Sequence);
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				RtpQualityHelper::GetInstance()->mUdpSocket.SendData((char*)(&timesync), len);
				RtpQualityHelper::GetInstance()->mRttBuf[RtpQualityHelper::GetInstance()->mRttIndex % RTTNUM].sequence = Sequence;
				RtpQualityHelper::GetInstance()->mRttBuf[RtpQualityHelper::GetInstance()->mRttIndex % RTTNUM].server_ime = timesync.serverTime;
				RtpQualityHelper::GetInstance()->mRttIndex++;*/
			}
			else if (timesync.mode == 1)
			{
				uint64_t servertime = GetTimestampUs();
				uint64_t last_server_time = RtpQualityHelper::GetInstance()->FindServerTime(timesync.sequence);
				uint64_t rtt = servertime - last_server_time;
				string msg = "TimeSync  mode 1 clientTime= " + to_string(timesync.clientTime) + ",serverTime=" + to_string(timesync.serverTime)
					+ ",locateTime=" + to_string(servertime) + ",rtt=" + to_string(rtt) + ",lastrtt=" + to_string(RtpQualityHelper::GetInstance()->mLastServerRtt) + ",last_server_time" +
					to_string(last_server_time) + "sequence" + to_string(timesync.sequence);
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				if (RtpQualityHelper::GetInstance()->mLastServerRtt != 0)
				{
					float rttChangeRate = (float)(rtt - RtpQualityHelper::GetInstance()->mLastServerRtt) / RtpQualityHelper::GetInstance()->mLastServerRtt;
					string msg = "rtt change rate " + to_string(rttChangeRate);
					GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				}
				RtpQualityHelper::GetInstance()->mLastServerRtt = rtt;
				if (gLog)
				{
					string msg = "rtt = " + to_string(rtt);
					GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				}
			}
			else if (timesync.mode == 2)
			{


			}
			uint64_t getlosttime = GetTimestampUs();
			if (timesync.type == 1)///left nack
			{
				u_short sequence = timesync.stepNumber;
				//sequence = ntohs(sequence);

				string msg = "getlost seqno= " + to_string(sequence) + "eye" + to_string(0) + "timest=" + to_string(getlosttime);
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				RtpQualityHelper::GetInstance()->GetAndSendLostRtpBuf(sequence, 0);
			}
			else if (timesync.type == 2)///right nack
			{
				uint64_t servertime = GetTimestampUs();
				u_short sequence = timesync.stepNumber;
				//sequence = ntohs(sequence);
				string msg = "getlost seqno= " + to_string(sequence) + "eye" + to_string(1) + "timest=" + to_string(getlosttime);;
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				RtpQualityHelper::GetInstance()->GetAndSendLostRtpBuf(sequence, 1);
			}
		}

	}

	return 1;
}
unsigned int __stdcall RtpQualityBitRateHelperThread(LPVOID lpParameter)
{
	while (true)
	{
		Sleep(10);
		RtpQualityHelper::GetInstance()->IncreaseBitRate();
	}
	return 1;
}


void RtpQualityHelper::SaveRtpBuf(char* buf, int buflen, int eyeIndex, bool makefec)
{///都存，makefec==ture 的makefec
	if (gConfig.GetFecValue()==0)
	{
		makefec = false;
	}
	 
	if (makefec)
	{
		if (eyeIndex == 0)
		{

			bool ret = mLeftFec->save_current_frame(buf, buflen);
			if (ret)
			{
				// send  fec

				int ret = mUdpSocket.SendData(mLeftFec->fec_buf_, mLeftFec->fec_buf_length_, mDstIp, mLeftPort, mLeftRtpSocket);
				if (ret < 0)
				{
					string msg = "mUdpSocket send left fec error " + to_string(ret);
					GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				}
			}
		}
		else
		{
			bool ret = mRightFec->save_current_frame(buf, buflen);
			if (ret)
			{
				// send  fec
				int ret = mUdpSocket.SendData(mRightFec->fec_buf_, mRightFec->fec_buf_length_, mDstIp, mRightPort, mRigthRtpSocket);
				if (ret < 0)
				{
					string msg = "mUdpSocket send right fec error " + to_string(ret);
					GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				}
			}
		}

	}

	if (gConfig.GetRtpAckValue()==0)
	{
		return;
	}

	/*RTP_FIXED_HEADER* rtp_head = (RTP_FIXED_HEADER*)buf;
	if (ntohs(rtp_head->seq_no) == 20655)
	{
		uint64_t timet = GetTimestampUs();
		string msg = "20655  testeyeindex" + to_string(eyeIndex) + to_string(timet);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
	}*/
	if (eyeIndex == 0)
	{
		if (mLeftRtpStore[mLeftRtpStoreIndex % STORE_BUF_NUM].buf_size == 1500)
		{//当前存满后，index下移到最近没有使用的storebuf，buf_size 清零并使用,刚存满的buf不清零
			mLeftRtpStoreIndex++;
			mLeftRtpStore[mLeftRtpStoreIndex % STORE_BUF_NUM].buf_size = 0;
			RTP_FIXED_HEADER* rtp_head = (RTP_FIXED_HEADER*)buf;
			mLeftRtpStore[mLeftRtpStoreIndex % STORE_BUF_NUM].begin_step_no = ntohs(rtp_head->seq_no);
		}
		int buf_index = mLeftRtpStore[mLeftRtpStoreIndex % STORE_BUF_NUM].buf_size;
		char* dst = mLeftRtpStore[mLeftRtpStoreIndex % STORE_BUF_NUM].buf[buf_index];
		memmove(dst, buf, buflen);
		mLeftRtpStore[mLeftRtpStoreIndex % STORE_BUF_NUM].buflen[buf_index] = buflen;
		mLeftRtpStore[mLeftRtpStoreIndex % STORE_BUF_NUM].buf_size++;

	}
	else
	{
		if (mRightRtpStore[mRightRtpStoreIndex % STORE_BUF_NUM].buf_size == 1500)
		{//当前存满后，index下移到最近没有使用的storebuf，buf_size 清零并使用,刚存满的buf不清零
			mRightRtpStoreIndex++;
			mRightRtpStore[mRightRtpStoreIndex % STORE_BUF_NUM].buf_size = 0;
			RTP_FIXED_HEADER* rtp_head = (RTP_FIXED_HEADER*)buf;
			mRightRtpStore[mRightRtpStoreIndex % STORE_BUF_NUM].begin_step_no = ntohs(rtp_head->seq_no);
		}
		int buf_index = mRightRtpStore[mRightRtpStoreIndex % STORE_BUF_NUM].buf_size;
		char* dst = mRightRtpStore[mRightRtpStoreIndex % STORE_BUF_NUM].buf[buf_index];
		memmove(dst, buf, buflen);
		mRightRtpStore[mRightRtpStoreIndex % STORE_BUF_NUM].buflen[buf_index] = buflen;
		mRightRtpStore[mRightRtpStoreIndex % STORE_BUF_NUM].buf_size++;
	}

}
bool RtpQualityHelper::GetAndSendLostRtpBuf(u_short lostNo, int eyeIndex)
{
	char buf[1500] = { 0 };
	int buflen = 0;
	bool find_flag = false;
	if (eyeIndex == 0)
	{
		for (int i = 0; i < STORE_BUF_NUM; i++)
		{

			if (lostNo >= mLeftRtpStore[i].begin_step_no && lostNo < mLeftRtpStore[i].begin_step_no + mLeftRtpStore[i].buf_size)
			{
				int index_in_storebuf = lostNo - mLeftRtpStore[i].begin_step_no;
				memmove(buf, mLeftRtpStore[i].buf[index_in_storebuf], mLeftRtpStore[i].buflen[index_in_storebuf]);
				buflen = mLeftRtpStore[i].buflen[index_in_storebuf];
				find_flag = true;
			}

		}
	}
	else
	{
		for (int i = 0; i < STORE_BUF_NUM; i++)
		{

			if (lostNo >= mRightRtpStore[i].begin_step_no && lostNo < mRightRtpStore[i].begin_step_no + mRightRtpStore[i].buf_size)
			{

				int index_in_storebuf = lostNo - mRightRtpStore[i].begin_step_no;
				memmove(buf, mRightRtpStore[i].buf[index_in_storebuf], mRightRtpStore[i].buflen[index_in_storebuf]);
				buflen = mRightRtpStore[i].buflen[index_in_storebuf];
				find_flag = true;
			}

		}
	}
	if (find_flag)
	{

		RTP_FIXED_HEADER* rtp_head = (RTP_FIXED_HEADER*)buf;
		string msg = "sendlost get find_flag " + to_string(find_flag) + "seqno" + to_string(ntohs(rtp_head->seq_no)) + "eye:" + to_string(eyeIndex) + "in=" + to_string(lostNo);
		uint64_t servertime = GetTimestampUs();
		msg = msg + "time=" + to_string(servertime);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
		SOCKET dstsocket = eyeIndex == 0 ? mLeftRtpSocket : mRigthRtpSocket;
		u_short port = eyeIndex == 0 ? gConfig.GetPortL() : gConfig.GetPortR();
		int ret = mUdpSocket.SendData(buf, buflen, mDstIp, port);
		servertime = GetTimestampUs();
		msg = "sendlost over  seqno" + to_string(ntohs(rtp_head->seq_no)) + "eye:" + to_string(eyeIndex) + "in=" + to_string(lostNo);
		servertime = GetTimestampUs();
		msg = msg + "time=" + to_string(servertime);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
		if (ret < 0)
		{
			string msg = "GetAndSendLostRtpBuf send fail  ";
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
			return false;
		}
	}
	else
	{
		string msg = "sendlost get find_flag failed " + to_string(find_flag) + "eye:" + to_string(eyeIndex) + "in=" + to_string(lostNo);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
	}

	return find_flag;
}

RtpQualityHelper* RtpQualityHelper::GetInstance()
{

	if (gRtpQualityHelperInstance == NULL)
	{
		gRtpQualityHelperInstance = new RtpQualityHelper();
		return gRtpQualityHelperInstance;
	}
	return gRtpQualityHelperInstance;

}

int RtpQualityHelper::SetSocketParam(SOCKET leftSocket, SOCKET rightSocket, std::string dstIp, u_short leftPort, u_short rightPort)
{
	mLeftRtpSocket = leftSocket;
	mRigthRtpSocket = rightSocket;
	mRightPort = rightPort;
	mLeftPort = leftPort;
	mDstIp = dstIp;
	mUdpSocket.InitSocket(dstIp, 29718);
	HANDLE   ret = (HANDLE)_beginthreadex(NULL, 0, &RtpQualitySendRttThread, 0, 0, NULL);
	ret = (HANDLE)_beginthreadex(NULL, 0, &RtpQualityNetHelperThread, 0, 0, NULL);
	ret = (HANDLE)_beginthreadex(NULL, 0, &RtpQualityBitRateHelperThread, 0, 0, NULL);
	return 1;
}
int RtpQualityHelper::Init()
{

	for (int i = 0; i < MAX_SAVE_BUF; i++)
	{
		for (int j = 0; j < STORE_BUF_NUM; j++)
		{
			mLeftRtpStore[j].buf[i] = new char[1500];
			mRightRtpStore[j].buf[i] = new char[1500];
		}

	}


	mLeftFec = new fec_packet;
	mLeftFec->init();
	mRightFec = new fec_packet;
	mRightFec->init();
	return 1;
}
uint64_t RtpQualityHelper::FindServerTime(uint64_t sequence)
{
	for (int i = 0; i < RTTNUM; i++)
	{
		if (sequence == mRttBuf[i].sequence)
		{
			/* string msg = "rtt=" + to_string(sequence) + " find";
			 GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);*/
			return mRttBuf[i].server_ime;
		}
	}
	string msg = "rtt=" + to_string(sequence) + "notfind";
	GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
	return 100;
}





 int RtpQualityHelper::SetBitRate(int averageRate, int maxRate) 
 {
	 minAverageRate=averageRate/5;
	 minMaxRate = maxRate/5;

	 maxAverageRate = averageRate;
	 maxMaxRate= maxRate;

	 curAverageRate= averageRate;
	 curMaxRate= maxRate;
	 mLastRateIncreaseTime = mLastRateIncreaseTime = GetTimestampUs() + 30000;
	 mLastRateDecreaseTime = GetTimestampUs() + 500000;
	 mRateIncreaseTimeSpace = 2000000;
     mRateDecreaseTimeSpace = 20000000;

	 return 1;
 }
 bool RtpQualityHelper::ChangeEncodeBitRate(int index, int& averageRate, int& maxRate, bool& itype)
 {
	 if (gConfig.GetAutoRateValue()==0)
	 {
		 return false;
	 }
	 bool ret = false;
	 if (curAverageRate < maxAverageRate / 3)
	 {
		 if (mIdrCount % 2 == 0)
		 {
			 mLeftIdr = true;
			 
		 }
		 else
		 {
			 mRightIdr = true;
		 }
		 mIdrCount++;	 
	 }
	 if (index==0)
	 {
		 ret = mLeftChangeRate;
		 itype = mLeftIdr;
		 mLeftChangeRate = false;
		 mLeftIdr = false;
	 } 
	 else
	 {
		 ret = mRightChangeRate;
		 itype = mRightIdr;
		 mRightChangeRate = false;
		 mRightIdr = false;
	 }
	 
	 averageRate = curAverageRate;
	 maxRate = curMaxRate;
	 
	 return ret;
 }
 void RtpQualityHelper::GetCurrentRate(int& averageRate, int& maxRate)
 {
	 averageRate = curAverageRate;
	 maxRate = curMaxRate;
 }

 void RtpQualityHelper::IncreaseBitRate() 
 {
	 //rtc 模式使用建议值，不自动增长
	 if (gConfig.GetRtcOrBulkMode_() == 1) 
	 {
		 return;
	 }
	 uint64_t timestamp = GetTimestampUs();
	 mIncreaseCount++;
	 if ((mIncreaseCount >= 10)&&((timestamp-mLastRateDecreaseTime)>3000000))
	 {
		 mIncreaseCount = 0;
		 maxAverageRate = maxAverageRate*5/4;
		 maxMaxRate = maxMaxRate * 5 / 4;
		 if (maxAverageRate > gConfig.GetAverageBitRateValue() )
		 {
			 maxAverageRate = gConfig.GetAverageBitRateValue() ;
		 }
		 if (maxMaxRate > gConfig.GetMaxBitRateValue())
		 {
			 maxMaxRate = gConfig.GetMaxBitRateValue() ;
		 }
	 }
	 if (curAverageRate == maxAverageRate)
	 {
		 return;
	 }
	 if ((timestamp - mLastRateIncreaseTime) > 3000000)
	 {
		 mRateIncreaseTimeSpace = mRateIncreaseTimeSpace -500000;
		 if (mRateIncreaseTimeSpace< 2000000)
		 {
			 mRateIncreaseTimeSpace = 2000000;
		 }
	 }
	 
	 if (((timestamp - mLastRateIncreaseTime) > mRateIncreaseTimeSpace)&&
		 ((timestamp-mLastRateDecreaseTime)>5000000))
	 {
		 curAverageRate = (int)((float)curAverageRate * 9 / 8);
		 curMaxRate = (int)((float)curMaxRate * 9 / 8);
		// mRateIncreaseTimeSpace=mRateIncreaseTimeSpace - 500000;
		 if (curMaxRate>maxMaxRate)
		 {
			 curMaxRate = maxMaxRate;
		 }
		 if (curAverageRate > maxAverageRate)
		 {
			 curAverageRate = maxAverageRate;
		 }
		 mLeftChangeRate = mRightChangeRate = true;
		// if (gLog)
		 {
			 string msg = "increase rate aver=" + to_string(curAverageRate) + "max=" + to_string(curMaxRate);
			 GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
			 mLastRateIncreaseTime = timestamp;
		 }
		
	 }
	
 }
 void RtpQualityHelper::DecreaseBitRate(float div, int flag)
 {
	 uint64_t timestamp = GetTimestampUs();
	 if (mDecreaseTimeTrace==0)
	 {
		 mDecreaseTimeTrace = timestamp;
	 }
	 mDecreaseCount++;
	 if (mDecreaseCount>20)
	 {
		
		 mDecreaseCount = 0;
		 maxAverageRate = maxAverageRate *2/3;
		 maxMaxRate = maxMaxRate * 2 / 3;
		 if (maxAverageRate<gConfig.GetAverageBitRateValue()/3)
		 {
			 maxAverageRate = gConfig.GetAverageBitRateValue() / 3;
		 }
		 if (maxMaxRate < gConfig.GetMaxBitRateValue() / 3)
		 {
			 maxMaxRate = gConfig.GetMaxBitRateValue() / 3;
		 }
		 string msg = "DecreaseBitRate over 5 in second" + to_string(maxAverageRate) + "max=" + to_string(maxMaxRate);
		 GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
	 }

	 if ((timestamp-mDecreaseTimeTrace)>5000000)
	 {
		 mDecreaseTimeTrace = 0;
		 mDecreaseCount = 0;
	 }
	 mDecreaseSecondCount++;
	 
	 if ((timestamp - mDecreaseSecondTimeTrace) > 1000000)
	 {
		 mDecreaseSecondTimeTrace = 0;
		 mDecreaseSecondCount = 0;
	 }
	 if ((mDecreaseSecondCount < 2)&&(flag!=(int)DecreaseFlag::high))
	 {
		 return;
	 }
	 if (curAverageRate==minAverageRate)
	 {
		 mLastRateDecreaseTime = timestamp;
		 return;
	 }
	 if (mLastRateDecreaseTime-mLastRateIncreaseTime<2000000)
	 {
		 mRateIncreaseTimeSpace = mRateIncreaseTimeSpace * 2;
	 }
	 if ((timestamp - mLastRateDecreaseTime)> mRateDecreaseTimeSpace)
	 {
		 mRateIncreaseTimeSpace = int(1.1f * (float)mRateIncreaseTimeSpace);
		 curAverageRate = (float)curAverageRate /div;
		 curMaxRate = (float)curMaxRate  / div;
		 if (curMaxRate < 900000)
		 {
			 curMaxRate = 900000;
		 }
		 if (curAverageRate<600000)
		 {
			 curAverageRate = 600000;
		 }
		 if (curMaxRate<minMaxRate)
		 {
			 curMaxRate = minMaxRate;
		 }
		 if (curAverageRate<minAverageRate)
		 {
			 curAverageRate = minAverageRate;
		 }
		 mLeftChangeRate=mRightChangeRate = true;
		// if (gLog)
		 {
			 string msg = "decrease rate aver=" + to_string(curAverageRate) + "max=" + to_string(curMaxRate);
			 GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
			 mLastRateDecreaseTime = timestamp;
		 }
		
	 }
	
 }
 void RtpQualityHelper::ForceMaxBitRate(int averageRate, int maxRate)
 {
	 SetBitRate(averageRate, maxRate);
	 mLeftChangeRate = mRightChangeRate = true;
 }