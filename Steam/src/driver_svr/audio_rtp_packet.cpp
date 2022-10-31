#include "audio_rtp_packet.h"
#include <winsock2.h>
#include <stdio.h>
#include <memory.h>
#include <WS2tcpip.h>
#include <map>
#include <process.h>
#include <windows.h>
#include "config_reader.h"
extern ConfigReader gConfigReader;
extern std::string gDstip;
extern bool g_save_audio;
int RtpPacket::InitSocket(u_short port,int sysnum)
{
	

	m_SocketClient = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_SocketClient == INVALID_SOCKET)
	{
		OutputDebugString(L"socket init error");
	}

	char	chOptVal = 1;
	if (setsockopt(m_SocketClient, SOL_SOCKET, SO_REUSEADDR, &chOptVal, sizeof(char)) == SOCKET_ERROR)
	{
		OutputDebugString(L"端口复用设置失败");
	}

	struct sockaddr_in Local_Addr;
	Local_Addr.sin_family = AF_INET;
	Local_Addr.sin_port = htons(port);
	m_iSendToPort =  port;
	Local_Addr.sin_addr.s_addr = INADDR_ANY; //inet_addr(ip);

	if (bind(m_SocketClient, (struct sockaddr *)&Local_Addr, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		OutputDebugString(L"绑定失败");
	}
	return 1;
	
}
RtpPacket::RtpPacket():m_nBufLen(0), m_unTimespaceCurrent(0), m_usSeqNum(0), m_iType(-1), m_SendAudio(0)
,m_iSysNum(6)
{
	
	m_unTimestampIncrese = (unsigned int)(5); //480 is  10ms ，split to  two packet ，then one of it is 5ms
}

void RtpPacket::GetClientMsg()
{
	char buffer[1024] = {0};
	int len = sizeof(m_Clientaddr);
	while (true)
	{
		int recvlen = recvfrom(m_SocketClient, buffer, sizeof(buffer), 0, (struct sockaddr*)&m_Clientaddr, &len);
		if (recvlen != SOCKET_ERROR)
		{
			unsigned char UDPHead = buffer[0];
			UDPHead = UDPHead &0xff;
			unsigned char UDPEnd = buffer[recvlen - 1];
			UDPEnd=	UDPEnd & 0xff;
			if (UDPHead != 0x13 || UDPEnd != 0xee)
			{
				continue;
			}
			char OperateCode=buffer[12] & 0x00ff;
			if (OperateCode==0x01)
			{
				m_SendAudio = true;
			} 
			else if(OperateCode == 0x00)
			{
				m_SendAudio = false;
				break;//?? 退出
			}
		}
	}
	
}

RtpPacket::~RtpPacket()
{
	closesocket(m_SocketClient);
}
//FILE *pf = fopen("out1.pcm", "wb+");
FILE * pSendAuido = NULL;
int RtpPacket::DoSendAudio(char * buf, int len, int Samples)
{
	/*if (m_SendAudio==false)
	{
		return 0;
	}*/
	int payloadStart = 12;//rtp头后一位。即负载开始的位置
	int sendBytes = 0;
	char* nalu_payload;
	unsigned long csrc = htonl(m_iSysNum);

	memset(m_SendBuf, 0, MAX_BUF_LEN);//清空m_SendBuf；此时会将上次的时间戳清空，因此需要m_unTimespaceCurrent来保存上次的时间戳值
	//rtp固定包头，为12字节,该句将m_SendBuf[0]的地址赋给rtp_hdr，以后对rtp_hdr的写入操作将直接写入m_SendBuf。
	rtp_hdr = (RTP_FIXED_HEADER*)&m_SendBuf[0];
	//设置RTP HEADER，
	rtp_hdr->payload = PCM;  //负载类型号，
	rtp_hdr->version = 2;  //版本号，此版本固定为2
    rtp_hdr->marker    =1;   //标志位，由具体协议规定其值。
	rtp_hdr->ssrc = csrc;    //随机指定为10，并且在本RTP会话中全局唯一
	rtp_hdr->extension = 0;
	nalu_payload = &m_SendBuf[payloadStart];
	memcpy(nalu_payload, &Samples, sizeof(int));
	nalu_payload = &m_SendBuf[payloadStart+ sizeof(int)];
	memcpy(nalu_payload,buf, len);
	rtp_hdr->seq_no = htons(m_usSeqNum++); //序列号，每发送一个RTP包增1

	rtp_hdr->timestamp = htonl(m_unTimespaceCurrent);
	sendBytes = payloadStart+len+ sizeof(int);						//获得m_SendBuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节
	m_Clientaddr.sin_port = htons(m_iSendToPort);
	m_Clientaddr.sin_addr.S_un.S_addr = inet_addr(gDstip.c_str());
	m_Clientaddr.sin_family = AF_INET;
	int ret = sendto(m_SocketClient, m_SendBuf, sendBytes, 0, (sockaddr *)&m_Clientaddr, sizeof(m_Clientaddr));
	
	if (g_save_audio)
	{
		if (pSendAuido==NULL)
		{
			pSendAuido = fopen("pico_send_audio.pcm", "wb+");
		}
		if (pSendAuido!=NULL)
		{
			fwrite(m_SendBuf + 16, sizeof(char), sendBytes - 16, pSendAuido);
		}
	}
  // fwrite(m_SendBuf+16,sizeof(char), sendBytes -16,pf);
	m_unTimespaceCurrent += m_unTimestampIncrese;
	return ret;
}

AudioSender::AudioSender():m_BufIndex(0), m_SendIndex(0),m_DelayBuffer(0)
{
}

AudioSender::~AudioSender()
{
	if (mRtpPakcet != nullptr)
	{
		delete mRtpPakcet;
	}
}

void AudioSender::InitAudioSender(u_short port, int sysnum)
{
	mRtpPakcet = new RtpPacket();
	mLoop = true;
	HANDLE ret;
	//ret = (HANDLE)_beginthreadex(NULL, 0, &SendAudioThread, this, 0, &mSendAudioThreadId);
	//ret = (HANDLE)_beginthreadex(NULL, 0, &GetMsgThread, this, 0, &mGetMsgThreadId);
	mRtpPakcet->InitSocket(port, sysnum);
	m_DelayBuffer = gConfigReader.GetAudioDelayTime() / 5;
}

unsigned int __stdcall AudioSender::GetMsgThread(LPVOID lpParameter)
{
	AudioSender *pSenderObj = (AudioSender*)lpParameter;
	pSenderObj->mRtpPakcet->GetClientMsg();//loop
	return 0;
}

unsigned int __stdcall AudioSender::SendAudioThread(LPVOID lpParameter)
{
	AudioSender *pSenderObj = (AudioSender*)lpParameter;
	MSG msg;
	PeekMessage(&msg, NULL, MSG_PACKET_AND_SEND, MSG_END_LOOP, PM_NOREMOVE);
	int sendIndex = 0;
	while (pSenderObj->mLoop)
	{
		GetMessage(&msg, 0, MSG_PACKET_AND_SEND, MSG_END_LOOP);
		switch (msg.message) 
		{
		case MSG_PACKET_AND_SEND: 
		{
			
			pSenderObj->mRtpPakcet->DoSendAudio(pSenderObj->mAudioBuf[sendIndex].buf, pSenderObj->mAudioBuf[sendIndex].len,0);
			sendIndex = (sendIndex + 1) % AUDIOBUFSIZE;
			break;
		}
		case MSG_END_LOOP: 
		{
			pSenderObj->mLoop = false;
			break;
		}
		}

	}
	return 0;
}
int GetSendBufSize(int all_length, int& buf_len, int& buf_count)
{
	int ret = 0;
	if (all_length % 882 == 0)
	{
		ret = 882;
	} 
	else if (all_length % 896 == 0)
	{
		ret = 896;
	}
	else if (all_length % 960 == 0)
	{
		ret = 960;
	}
	else if (all_length % 1024 == 0)
	{
		ret = 1024;
	}
	else if (all_length < 1400)
	{
		ret = all_length;
	}
	else
	{
		ret = 1400;
	}
	buf_len = ret;
	buf_count = all_length / buf_len + ((all_length % buf_len == 0) ? 0 : 1);
	return ret;
}
void AudioSender::SendAudioBuf(char * buf, int len, int Samples)
{
	int packetcount = 0;
	int buflen = 0;
	 GetSendBufSize(len, buflen,packetcount);
 
	char packetbuf[1024] = { 0 };
	
	int all_len = len;
	int used_len = 0;
	for (int i = 0; i < packetcount; i++)
	{
		if (all_len> buflen)
		{
			all_len -= buflen;
		}
		else
		{
			buflen = all_len;
		}
	
		memset(mAudioBuf[m_BufIndex].buf, 0, SENDBUFSIZE);
		
		memcpy(mAudioBuf[m_BufIndex].buf, buf +used_len, buflen);
		mAudioBuf[m_BufIndex].len = buflen;
		used_len += buflen;
		if (abs(m_BufIndex-m_SendIndex)>= m_DelayBuffer)
		{
			 
			mRtpPakcet->DoSendAudio(mAudioBuf[m_SendIndex].buf, mAudioBuf[m_SendIndex].len,Samples);
			m_DelayBuffer = 0;
			m_SendIndex = (m_SendIndex + 1) % AUDIOBUFSIZE;
		}	
		m_BufIndex = (m_BufIndex + 1) % AUDIOBUFSIZE;
		
		//PostThreadMessage(mSendAudioThreadId, MSG_PACKET_AND_SEND, NULL, NULL);
	}

	
	
}
AudioSender* AudioSender::mInstance = nullptr;
AudioSender * AudioSender::GetInstance()
{

	if (nullptr == mInstance)
	{
		mInstance = new AudioSender();
	}

	return mInstance;
}


AudioReceiver* AudioReceiver::mInstance = nullptr;
AudioReceiver* AudioReceiver::GetInstance()
{

	if (nullptr == mInstance)
	{
		mInstance = new AudioReceiver();
	}

	return mInstance;
}

bool AudioReceiver::InitAudioReceiver(u_short port) 
{
	socket_ = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_ == INVALID_SOCKET)
	{
		OutputDebugString(L"socket init error");
	}

	char	chOptVal = 1;
	if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &chOptVal, sizeof(char)) == SOCKET_ERROR)
	{
		OutputDebugString(L"端口复用设置失败");
	}
 
	server_addr_.sin_family = AF_INET;
	server_addr_.sin_port = htons(port);
	locate_port_ = port;
	server_addr_.sin_addr.s_addr = INADDR_ANY; //inet_addr(ip);

	if (bind(socket_, (struct sockaddr*)&server_addr_, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		OutputDebugString(L"绑定失败");
	}
	return 1;
}

int AudioReceiver::RecvBuf(char *buf,int buf_len) 
{
	int len = sizeof(struct sockaddr);
	struct sockaddr_in addr;
	int recvRet = recvfrom(socket_,  buf, buf_len, 0,(sockaddr*)&addr, &len);
	std::string dst_ip = inet_ntoa(addr.sin_addr);
	if (dst_ip.compare(gDstip)!=0)
	{
		return 0;
	}
	return recvRet;
}
void AudioReceiver::ShutDown() 
{
	closesocket(socket_);
}