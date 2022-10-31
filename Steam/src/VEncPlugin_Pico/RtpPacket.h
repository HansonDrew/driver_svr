#pragma once
#include<WinSock2.h>
#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>
#include <string>
#include "fec_define.h"

//#define RTPSAVE
#define MSG_SOCKET_SEND WM_USER+109
#define MSG_RECONNECT WM_USER+111

//#define THREADSEND
#define MSG_FRAME_SENDOVER WM_USER+110
#define SENDBFUSIZE 1000
#define PACKET_BUFFER_END            (unsigned int)0x00000000
//数据链路层 mtu 包含 ip头和 udp头  20ip头+8udp头 +12rtp +4 扩展标识+48扩展+数据<1450
//#define MAX_BUF_LEN 1052
//#define MAX_RTP_PKT_LENGTH     949//1350

#define MAX_BUF_LEN 1450
#define MAX_RTP_PKT_LENGTH     1356//1336

#define H264                    96
#define H265                    98
struct AddQuaternion
{
	float x, y, z, w;
};
struct AddVector3
{
	float x, y, z;
};


struct AddPoseData
{
	AddQuaternion rotation;
	AddVector3 position;

	uint64_t poseTimeStamp;
	float predictedTimeMs;
	uint64_t hmdPoseTimeTs;
	int gameRenderCost;
	int encodeCost;
	int autoRateFlag;
	int encodeRate;
};


typedef struct {
	//byte 0
	unsigned char TYPE : 6;
	unsigned char E : 1;
	unsigned char S : 1;
} H265_FU_HEADER; /**//* 1 BYTES */
typedef struct
{
	unsigned short paddingprofile;
	unsigned short  paddinglen;   //reallen=paddinglen*4 byte  （32bits）  
}RTP_PADDING_HEAD;

typedef struct {
	//byte 0
	unsigned char TYPE : 5;
	unsigned char NRI : 2;
	unsigned char F : 1;

} NALU_HEADER; /**//* 1 BYTES */

typedef struct {
	//byte 0
	unsigned char TYPE : 5;
	unsigned char NRI : 2;
	unsigned char F : 1;


} FU_INDICATOR; /**//* 1 BYTES */

typedef struct {
	//byte 0
	unsigned char TYPE : 5;
	unsigned char R : 1;
	unsigned char E : 1;
	unsigned char S : 1;
} FU_HEADER; /**//* 1 BYTES */


typedef struct
{
	int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
	unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
	unsigned max_size;            //! Nal Unit Buffer size
	int forbidden_bit;            //! should be always FALSE
	int nal_reference_idc;        //! NALU_PRIORITY_xxxx
	int nal_unit_type;            //! NALU_TYPE_xxxx    
	char *buf;                    //! contains the first byte followed by the EBSP
	unsigned short lost_packets;  //! true, if packet loss is detected
} NALU_t;
NALU_t *AllocNALU(int buffersize);

void FreeNALU(NALU_t *n);
enum rtpcongtype
{
	 fps=0,
	 codetype=1
};
#define INIT_VALUE 0x0
#define FINAL_VALUE 0x0

class CrcDevice
{
public:
	CrcDevice()
	{
	}
	~CrcDevice()
	{
	}

	int isTableInit = 0;
	uint32_t table[256];
	uint32_t POLYNOMIAL = 0x1EDC6F41;;
	void initTable() {
		isTableInit = 1;		
		uint32_t crc;
		for (int i = 0; i < 256; i++) {
			crc = i << 24;
			for (int j = 0; j < 8; j++) {
				if ((crc & 0x80000000) != 0) {
					crc = POLYNOMIAL ^ (crc << 1);
				}
				else {
					crc = crc << 1;
				}
			}
			table[i] = crc;
		}
	}

	uint32_t crc32_table(uint8_t* p_buf, uint8_t* p_end) {
		if (isTableInit == 0) {
			initTable();
		}
		uint32_t crc = INIT_VALUE;
		while (p_buf < p_end) {
			/* 和直接求解类似，只不过把crc拆成了两部分，
			   变为了FF000000和00FFFFFF的形式，其中，后面一个的CRC值就是其本身，而前面的值可以查表得到 */
			uint32_t data = (uint32_t)*p_buf++;
			crc = (crc << 8) ^ table[(crc >> 24) & 0x000000FF] ^ table[data];
			// 根据计算的性质，上面的语句可以改写为下面的语句
			// crc = (crc << 8) ^ table[( (crc>>24) ^ data) & 0x000000FF];
		}
		return crc ^ FINAL_VALUE;
	}
};

class RtpPacket
{
public:
	bool mSendLoop;
	unsigned int mSendThreadId;
	static unsigned int __stdcall SendThread(LPVOID lpParameter);
	char *mSendBufQueue[SENDBFUSIZE];
	int mSendBufLenQueue[SENDBFUSIZE];
	char *mAddData[SENDBFUSIZE];
	int mSendBufIndex;
	int m_RtpHeadPaddingRealLen;//reallen=paddinglen*4 byte  （32bits）  an byte  计算（一个char）
	RTP_FIXED_HEADER        *rtp_hdr;
	RTP_PADDING_HEAD *rtp_pad_hdr;
	NALU_HEADER		*nalu_hdr;
	FU_INDICATOR	*fu_ind;
	FU_HEADER		*fu_hdr;
	H265_FU_HEADER		*fu_265hdr;
	static RtpPacket* GetInstance(int IndexFlag);
	static RtpPacket* CreateInstance(int IndexFlag);
	void SetConfig(rtpcongtype type,int idata);
	void SetDstIp(std::string ip);
	int mFps;
	unsigned int m_unTimespaceCurrent;
	unsigned int m_unTimestampIncrese = 0;
	CrcDevice crc_device_;
	int mEyeIndex;
	class GC // 垃圾回收类
	{
	public:
		GC()
		{
		}
		~GC()
		{
			// We can destory all the resouce here, eg:db connector, file handle and so on
			if (gLeftInstance != NULL)
			{
				delete gLeftInstance;
				gLeftInstance = NULL;
			}
			if (gRightInstance != NULL)
			{
				delete gRightInstance;
				gRightInstance = NULL;
			}
		}
	};
	static GC leftgc;  //垃圾回收类的静态成员
	static GC rightgc;  //垃圾回收类的静态成员
	SOCKET m_SocketClient;
	sockaddr_in m_Dst_Addr;
	int InitSocket(std::string ip, u_short port, int sysnum);
	FILE* pf = NULL;
	RtpPacket();
	NALU_t *m_Mynalu_t;
	char m_SendBuf[MAX_BUF_LEN];
	int m_nBufLen;
	
	void DoPacketH264(uint8_t*inputFrame, int len, int RtpAdding, uint8_t*rtpAddData, unsigned short addLen,bool frameEnd);
	void DoPacketH265(uint8_t*inputFrame, int len, int RtpAdding, uint8_t*rtpAddData, unsigned short addLen, bool frameEnd);
	void DoPacket(uint8_t*inputFrames, int len, int ifH264, int RtpAdding, uint8_t*rtpAddData, unsigned short addLen);
	unsigned short m_usSeqNum;
	~RtpPacket();
	void SetSysNum(int sysnum) { m_iSysNum = sysnum; };
	void SetType(int ntype) { m_iType = ntype; };
	int GetType() { return m_iType; };
	void SetSliceMode(bool slicemode) { m_bSliceMode = slicemode; };
	u_short socket_port;
private:
	int init_flag_ = false;
	static RtpPacket * gLeftInstance;
	static RtpPacket * gRightInstance;
	int m_iSysNum;
	int m_iType;
	bool m_bSliceMode;
};

class TcpPacket
{
public:

	static TcpPacket* GetInstance(int IndexFlag);
	static TcpPacket* CreateInstance(int IndexFlag);

	int mEyeIndex;
	
	
	class GC // 垃圾回收类
	{
	public:
		GC()
		{
		}
		~GC()
		{
			// We can destory all the resouce here, eg:db connector, file handle and so on
			if (gLeftTcpInstance != NULL)
			{
				delete gLeftTcpInstance;
				gLeftTcpInstance = NULL;
			}
			if (gRightTcpInstance != NULL)
			{
				delete gRightTcpInstance;
				gRightTcpInstance = NULL;
			}
		}
	};
	static GC leftTcpgc;  //垃圾回收类的静态成员
	static GC rightTcpgc;  //垃圾回收类的静态成员
	SOCKET m_SocketClient;
	SOCKADDR_IN dst_addr_;
	bool m_Connected=false;
	void SetDstIp(std::string ip);
	static unsigned int __stdcall ReconnectThread(LPVOID lpParameter);
	int InitSocket(std::string ip, u_short port);
	int SendToServer(uint8_t*video,int len, uint8_t*  pose,uint8_t ploadType, uint8_t eyeIndex=0xff);
	int ReConnectServer();
	int MulSend(char* buf, int len,int eyeIndex);
	void DoPacket(uint8_t* inputFrames, int len, int ifH264 , int RtpAdding , uint8_t* rtpAddData, unsigned short addLen, uint8_t eyeIndex = 0xff);
	FILE* pf = NULL;
	TcpPacket() { init_flag_ = false; };
	TcpPacket* m_Mynalu_t;
	char mSendBuf[6 * 1024 * 1024];
	char mOtherSendBuf[6 * 1024 * 1024];
	int m_nBufLen;
	unsigned mReconnectThreadId;

	std::string mIp;
	int mPort;

	unsigned short m_usSeqNum;
	~TcpPacket() {};
	uint64_t mConnectTime = 0;
	u_short socket_port;
private:
	int init_flag_ = false;
	static TcpPacket* gLeftTcpInstance;
	static TcpPacket* gRightTcpInstance;
	std::string dst_ip_;
	u_short dst_port_;
};


class SendIndex
{
public:
	SendIndex();
	~SendIndex();
	int GetInedx();
	void AddIndex();
	int left_send_thread_id_;
	int right_send_thread_id_;
private:
	int mSendIndex = -1;
	CRITICAL_SECTION mCSIndex;
};
