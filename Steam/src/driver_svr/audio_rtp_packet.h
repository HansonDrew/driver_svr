#pragma once
#include<WinSock2.h>
#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>

#define  MSG_PACKET_AND_SEND WM_USER+102
#define  MSG_END_LOOP WM_USER+106
#define PACKET_BUFFER_END            (unsigned int)0x00000000
#define MAX_BUF_LEN 1450
#define MAX_RTP_PKT_LENGTH     1410

#define H264                    96
#define H265                    96
#define PCM                     97
#define AUDIOBUFSIZE     200
#define SENDBUFSIZE 1024

typedef struct
{
	/**//* byte 0 */
	unsigned char csrc_len : 4;        /**//* expect 0 */
	unsigned char extension : 1;        /**//* expect 1, see RTP_OP below */
	unsigned char padding : 1;        /**//* expect 0 */
	unsigned char version : 2;        /**//* expect 2 */
	/**//* byte 1 */
	unsigned char payload : 7;        /**//* RTP_PAYLOAD_RTSP */
	unsigned char marker : 1;        /**//* expect 1 */
	/**//* bytes 2, 3 */
	unsigned short seq_no;
	/**//* bytes 4-7 */
	unsigned  long timestamp;
	/**//* bytes 8-11 */
	unsigned long ssrc;            /**//* stream number is used here. */
} RTP_FIXED_HEADER;
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
	char* buf;                    //! contains the first byte followed by the EBSP
	unsigned short lost_packets;  //! true, if packet loss is detected
} NALU_t;


class RtpPacket
{
public:

	
	RTP_FIXED_HEADER        *rtp_hdr;
	RTP_PADDING_HEAD *rtp_pad_hdr;
	NALU_HEADER		*nalu_hdr;
	FU_INDICATOR	*fu_ind;
	FU_HEADER		*fu_hdr;
	H265_FU_HEADER		*fu_265hdr;

	NALU_t *AllocNALU(int buffersize)
	{
		NALU_t *n;

		if ((n = (NALU_t*)calloc(1, sizeof(NALU_t))) == NULL)
		{
			return NULL;
		}

		n->max_size = buffersize;

		if ((n->buf = (char*)calloc(buffersize, sizeof(char))) == NULL)
		{
			free(n);
			return NULL;
		}

		return n;
	}
	//释放
	void FreeNALU(NALU_t *n)
	{
		if (n)
		{
			if (n->buf)
			{
				free(n->buf);
				n->buf = NULL;
			}
			free(n);
		}
	}
	int m_RtpHeadPaddingRealLen;//reallen=paddinglen*4 byte  （32bits）  an byte  计算（一个char）
	SOCKET m_SocketClient;
	struct sockaddr_in m_Clientaddr;
	int InitSocket( u_short port, int sysnum);
	RtpPacket();
	NALU_t *m_Mynalu_t;
	char m_SendBuf[MAX_BUF_LEN];
	int m_nBufLen;
	unsigned int m_unTimespaceCurrent;
	unsigned int m_unTimestampIncrese = 0;
	void GetClientMsg();
	bool m_SendAudio;
	unsigned short m_usSeqNum;
	~RtpPacket();
	void SetSysNum(int sysnum) { m_iSysNum = sysnum; };
	void SetType(int ntype) { m_iType = ntype; };
	int GetType() { return m_iType; };
	int DoSendAudio(char* buf, int len, int Samples);
private:
	int m_iSysNum;
	int m_iType;
	u_short m_iSendToPort;
};
typedef struct _AudioBuf
{
	char buf[SENDBUFSIZE];
	int len;
}AudioBuf;
class AudioSender
{
public:
	AudioSender();
	~AudioSender();
	void InitAudioSender(u_short port, int sysnum);
	RtpPacket *mRtpPakcet;
	unsigned  mGetMsgThreadId;
	unsigned mSendAudioThreadId;
	static unsigned int __stdcall GetMsgThread(LPVOID lpParameter);
	static unsigned int __stdcall SendAudioThread(LPVOID lpParameter);
	bool mLoop;
	void SendAudioBuf(char *buf ,int len,int Samples);
	AudioBuf mAudioBuf[AUDIOBUFSIZE];
	static AudioSender* GetInstance();
	int m_BufIndex;
	int m_SendIndex;
	int m_DelayBuffer;
	class GC
	{
	public:
		GC()
		{
		}
		~GC()
		{
			if (mInstance != NULL)
			{
				delete mInstance;
				mInstance = NULL;
			}
		}
	};
	static GC gc;  //垃圾回收类的静态成员
private:
	static AudioSender* mInstance;
};

class AudioReceiver
{
public:
	AudioReceiver() {};
	~AudioReceiver();
	bool InitAudioReceiver(u_short port);
	class GC
	{
	public:
		GC()
		{
		}
		~GC()
		{
			if (mInstance != NULL)
			{
				delete mInstance;
				mInstance = NULL;
			}
		}
	};
	static GC gc;  //垃圾回收类的静态成员
	static AudioReceiver* GetInstance();
	int RecvBuf(char* buf, int buf_len);
	void ShutDown();
private:
	SOCKET socket_;
	struct sockaddr_in server_addr_;
	u_short locate_port_;
	static AudioReceiver* mInstance;
};

