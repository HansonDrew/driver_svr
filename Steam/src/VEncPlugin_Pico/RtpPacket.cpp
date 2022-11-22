#include "RtpPacket.h"
#include <stdio.h>
#include <memory.h>
#include <map>
#include <process.h>
#include "../pxrTool/TimeTool.h"
#include "../../GlobalDLLContext.h"
#include "../pxrTool/TimeTool.h"
#include "../pxrTool/config_reader.h"
#include "fec_define.h"
#include "RtpQualityHelper.h"
extern int gInsertIdr;
extern int gEncodeType;
extern int gRtpPloadTpye;
 
RtpPacket* RtpPacket::gLeftInstance = nullptr;

RtpPacket::GC RtpPacket::leftgc;

RtpPacket* RtpPacket::gRightInstance = nullptr;
RtpPacket::GC RtpPacket::rightgc;
extern ConfigReader gConfig;

SendIndex gSendIndex;
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

unsigned int __stdcall RtpPacket::SendThread(LPVOID lpParameter)
{
	RtpPacket *RtpObj = (RtpPacket*)lpParameter;
	MSG msg;
	int BufferIndex = 0;
	 
	PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
	int bufinedx = 0;
	bool frame_begin = false;
	uint64_t begin_time=0;
	uint64_t end_time = 0;
	while (RtpObj->mSendLoop)
	{
		if (GetMessage(&msg, nullptr, MSG_SOCKET_SEND-1, MSG_FRAME_SENDOVER+1)) //get msg from message queue
		{
			switch (msg.message)
			{
			case MSG_SOCKET_SEND:
			{
				/// <rtp mark test>
				RTP_FIXED_HEADER rtp_head;
				memmove((void*)&rtp_head, RtpObj->mSendBufQueue[BufferIndex % SENDBFUSIZE],12);
				if (rtp_head.marker) 
				{
					frame_begin = true;
					end_time = nowInNs();
					if (begin_time != 0) 
					{
						std::string msg = "port =" + std::to_string(RtpObj->socket_port);
						msg += "sockettime="+std::to_string(float(end_time-begin_time)/1000000.000f);
						GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
					}
				}
				else 
				{
					if (frame_begin == true)
					{
						frame_begin = false;
						begin_time= nowInNs();
					}
				}

				/// <returns></returns>

				int ret = sendto(RtpObj->m_SocketClient, RtpObj->mSendBufQueue[BufferIndex%SENDBFUSIZE],
					RtpObj->mSendBufLenQueue[BufferIndex%SENDBFUSIZE], 0, (sockaddr *)&RtpObj->m_Dst_Addr, sizeof(RtpObj->m_Dst_Addr));
				BufferIndex++;
				/*for (int i=0;i<100;i++)
				{					 
					float a = 3.1415926f;
					float b = 3.1415926f;
					a* b;
				}*/
				break;
			}
			case MSG_FRAME_SENDOVER: 
			{
				break;
			}				
			}
		}
	}
	return 1;
}

RtpPacket * RtpPacket::GetInstance(int IndexFlag)
{
	if (IndexFlag==0)
	{
		if (gLeftInstance==NULL)
		{
			return CreateInstance(0);
		}
		return gLeftInstance;
	}
	else
	{
		if (gRightInstance == NULL)
		{
			return CreateInstance(1);
		}
		return gRightInstance;
	}
	return nullptr;
}

RtpPacket * RtpPacket::CreateInstance(int IndexFlag)
{
	if (IndexFlag==0)
	{
		gLeftInstance = new RtpPacket();
		gLeftInstance->mEyeIndex = 0;
	} 
	else
	{
		gRightInstance = new RtpPacket();
		gRightInstance->mEyeIndex = 1;
	}
	return nullptr;
}

void RtpPacket::SetConfig(rtpcongtype type, int idata)
{
	switch (type)
	{
	case fps:
		mFps = idata;
		break;
	case codetype:
		break;
	default:
		break;
	}
}

void RtpPacket::SetDstIp(std::string ip)
{
	string msg = "udp set_dst_ip:" + ip + "port:" + to_string(socket_port);
	GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
	m_Dst_Addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
}

int RtpPacket::InitSocket(std::string ip,u_short port,int sysnum)
{
#ifdef RTPSAVE
	
	if (mEyeIndex == 0)
	{
		pf = fopen("left.rtp", "wb+");
	}
	else
	{
		pf = fopen("ritht.rtp", "wb+");
	}
#endif	
	if (init_flag_)
	{
		return 1;
	}
	socket_port = port;
	for (int i = 0; i < SENDBFUSIZE; i++)
	{
		mSendBufQueue[i] = new char[MAX_BUF_LEN];
		mAddData[i] = new char[100];
	}
	 
	 
	HANDLE ret = (HANDLE)_beginthreadex(NULL, 0, &SendThread, this, 0, &mSendThreadId);
	SetThreadPriority(ret, THREAD_PRIORITY_TIME_CRITICAL);
	if (ret != NULL)
	{
		CloseHandle(ret);
	}
	float framerate = float(mFps);
	m_unTimestampIncrese = (unsigned int)(90000.0 / framerate);
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	WSAStartup(socketVersion, &wsaData);
	
	m_SocketClient = socket(AF_INET, SOCK_DGRAM, 0);
	
	if (m_SocketClient == INVALID_SOCKET)
	{
		//OutputDebugString(L"socket init error");
		return -1;
	}
	/*char	chOptVal = 1;
	if (setsockopt(m_SocketClient, SOL_SOCKET, SO_REUSEADDR, &chOptVal, sizeof(char)) == SOCKET_ERROR)
	{
		//OutputDebugString(L"端口复用设置失败");
	}*/	
	int value =8* 1024 * 1024 ;
	setsockopt(m_SocketClient, SOL_SOCKET, SO_SNDBUF, (char*)&value, sizeof(value));
	 
	m_Dst_Addr.sin_family = AF_INET;
	m_Dst_Addr.sin_port = htons(port);	  
	m_Dst_Addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());	 
	mSendLoop = true;	
	init_flag_ = true;
	return 1;	
}
RtpPacket::RtpPacket()
{
	m_Mynalu_t= AllocNALU(10*1024*1024);
	float framerate = 72;
	m_unTimestampIncrese = (unsigned int)(90000.0 / framerate);
	m_nBufLen = 0; 
	m_unTimespaceCurrent = 0; 
	m_usSeqNum = 0;
	m_iType = 0;
	crc_device_;
	init_flag_ = false;
}

void RtpPacket::DoPacketH264(uint8_t * inputFrame, int len, int RtpAdding = 0, uint8_t * rtpAddData = NULL, unsigned short addLen = 0,bool frameEnd=true)
{
	int payloadStart = 12;//rtp头后一位。即负载开始的位置
	int bytes = 0;
	char* nalu_payload;
	unsigned long csrc = htonl(m_iSysNum);
	m_Mynalu_t->len = len;
	if (len==0|| m_Mynalu_t->len==0)
	{
		return;
	}
	memmove(m_Mynalu_t->buf, inputFrame, len);
	m_Mynalu_t->forbidden_bit = m_Mynalu_t->buf[0] & 0x80; //1 bit
	m_Mynalu_t->nal_reference_idc = m_Mynalu_t->buf[0] & 0x60; // 2 bit
	m_Mynalu_t->nal_unit_type = (m_Mynalu_t->buf[0]) & 0x1f;// 5 bit

	memset(m_SendBuf, 0, MAX_BUF_LEN);//清空m_SendBuf；此时会将上次的时间戳清空，因此需要m_unTimespaceCurrent来保存上次的时间戳值
	//rtp固定包头，为12字节,该句将m_SendBuf[0]的地址赋给rtp_hdr，以后对rtp_hdr的写入操作将直接写入m_SendBuf。
	rtp_hdr = (RTP_FIXED_HEADER*)&m_SendBuf[0];
	//设置RTP HEADER，
	rtp_hdr->payload = H264;  //负载类型号，
	rtp_hdr->version = 2;  //版本号，此版本固定为2
	rtp_hdr->marker    = 0;   //标志位，由具体协议规定其值。
	rtp_hdr->ssrc = csrc;    //随机指定为10，并且在本RTP会话中全局唯一
	if (RtpAdding == 1)
	{
		rtp_hdr->extension = 1;
		payloadStart += 4;//  sizeof  RTP_PADDING_HEAD
		rtp_pad_hdr = (RTP_PADDING_HEAD*)&m_SendBuf[12];
		rtp_pad_hdr->paddingprofile = 0;
		m_RtpHeadPaddingRealLen = addLen;
		payloadStart += m_RtpHeadPaddingRealLen;
		unsigned short rtppaddinglen = addLen / 4; //4 byte =32 bits
		rtp_pad_hdr->paddinglen = htons(rtppaddinglen);
		memmove(&m_SendBuf[16], (void*)rtpAddData, addLen);
	}
	rtp_hdr->timestamp = htonl(m_unTimespaceCurrent);
	//	当一个NALU小于1400字节的时候，采用一个单RTP包发送
	if (m_Mynalu_t->len <= MAX_RTP_PKT_LENGTH)
	{
		int iRet;
		iRet = inputFrame[0] & 0x1f;
		//设置rtp M 位；
		if (iRet == 6 || iRet == 7 || iRet == 8||iRet==9)
		{
			rtp_hdr->marker = 0;
			m_unTimespaceCurrent = m_unTimespaceCurrent;
		}
		else
		{
			if (frameEnd)
			{
				rtp_hdr->marker = 1;
				m_unTimespaceCurrent = m_unTimespaceCurrent + m_unTimestampIncrese;
			}			
			
		}
		//设置NALU HEADER,并将这个HEADER填入m_SendBuf[12]
		nalu_hdr = (NALU_HEADER*)&m_SendBuf[payloadStart]; //将m_SendBuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入m_SendBuf中；
		nalu_hdr->F = m_Mynalu_t->forbidden_bit;
		nalu_hdr->NRI = m_Mynalu_t->nal_reference_idc >> 5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
		nalu_hdr->TYPE = m_Mynalu_t->nal_unit_type;

		nalu_payload = &m_SendBuf[payloadStart + 1];//同理将m_SendBuf[13]赋给nalu_payload
		memcpy(nalu_payload, m_Mynalu_t->buf + 1, m_Mynalu_t->len - 1);//去掉nalu头的nalu剩余内容写入m_SendBuf[13]开始的字符串。
		rtp_hdr->seq_no = htons(m_usSeqNum++); //序列号，每发送一个RTP包增1
	
		bytes = m_Mynalu_t->len + payloadStart;						//获得m_SendBuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节
		
		if (gConfig.GetCrc_() == 1)
		{
			int crc = crc_device_.crc32_table((uint8_t*)(m_SendBuf + payloadStart), (uint8_t*)(m_SendBuf + payloadStart+ m_Mynalu_t->len));
			AddPoseData* add_data = (AddPoseData*)&m_SendBuf[16];
			memmove((void*)&add_data->predictedTimeMs, &crc, sizeof(int));
		}

		int ret = sendto(m_SocketClient, m_SendBuf, bytes, 0, (sockaddr *)&m_Dst_Addr, sizeof(m_Dst_Addr));
		RtpQualityHelper::GetInstance()->SaveRtpBuf(m_SendBuf, bytes, mEyeIndex, true);
	}

	else if (m_Mynalu_t->len > MAX_RTP_PKT_LENGTH)
	{
		//得到该nalu需要用多少长度为1400字节的RTP包来发送
		int k = 0, l = 0;
		k = (m_Mynalu_t->len-1) / MAX_RTP_PKT_LENGTH;//需要k个1400字节的RTP包
		l = (m_Mynalu_t->len-1)%MAX_RTP_PKT_LENGTH;//最后一个RTP包的需要装载的字节数

		int t = 0;//用于指示当前发送的是第几个分片RTP包
		
		rtp_hdr->timestamp = htonl(m_unTimespaceCurrent);
		while (t <= k)
		{
			rtp_hdr->seq_no = htons(m_usSeqNum++); //序列号，每发送一个RTP包增1
			if (!t)//发送一个需要分片的NALU的第一个分片，置FU HEADER的S位
			{
				//设置rtp M 位；
				rtp_hdr->marker = 0;
				//设置FU INDICATOR,并将这个HEADER填入m_SendBuf[12]
				fu_ind = (FU_INDICATOR*)&m_SendBuf[payloadStart]; //将m_SendBuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入m_SendBuf中；
				fu_ind->F = m_Mynalu_t->forbidden_bit;
				fu_ind->NRI = m_Mynalu_t->nal_reference_idc >> 5;
				fu_ind->TYPE = 28;

				//设置FU HEADER,并将这个HEADER填入m_SendBuf[13]
				fu_hdr = (FU_HEADER*)&m_SendBuf[payloadStart + 1];
				fu_hdr->E = 0;
				fu_hdr->R = 0;
				fu_hdr->S = 1;
				fu_hdr->TYPE = m_Mynalu_t->nal_unit_type;
				nalu_payload = &m_SendBuf[payloadStart + 2];//同理将m_SendBuf[14]赋给nalu_payload
				memcpy(nalu_payload, m_Mynalu_t->buf + 1, MAX_RTP_PKT_LENGTH);//去掉NALU头
				bytes = MAX_RTP_PKT_LENGTH + payloadStart + 2;						//获得m_SendBuf的长度,为nalu的长度（除去起始前缀和NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节
				if (gConfig.GetCrc_() == 1)
				{
					int crc = crc_device_.crc32_table((uint8_t*)(m_SendBuf + payloadStart), (uint8_t*)(m_SendBuf + payloadStart + MAX_RTP_PKT_LENGTH+2));
					AddPoseData* add_data = (AddPoseData*)&m_SendBuf[16];
					memmove((void*)&add_data->predictedTimeMs, &crc, sizeof(int));
				}
				int ret = sendto(m_SocketClient, m_SendBuf, bytes, 0, (sockaddr *)&m_Dst_Addr, sizeof(m_Dst_Addr));
				RtpQualityHelper::GetInstance()->SaveRtpBuf(m_SendBuf, bytes, mEyeIndex, true);
				t++;

			}
			//发送一个需要分片的NALU的非第一个分片，清零FU HEADER的S位，如果该分片是该NALU的最后一个分片，置FU HEADER的E位
			else if (k == t)//发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节（当l>1386时）。
			{

				//设置rtp M 位；当前传输的是最后一个分片时该位置1
				if (frameEnd)
				{
					rtp_hdr->marker = 1;
					m_unTimespaceCurrent = m_unTimespaceCurrent + m_unTimestampIncrese;
				}
				//设置FU INDICATOR,并将这个HEADER填入m_SendBuf[12]
				fu_ind = (FU_INDICATOR*)&m_SendBuf[payloadStart]; //将m_SendBuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入m_SendBuf中；
				fu_ind->F = m_Mynalu_t->forbidden_bit;
				fu_ind->NRI = m_Mynalu_t->nal_reference_idc >> 5;
				fu_ind->TYPE = 28;

				//设置FU HEADER,并将这个HEADER填入m_SendBuf[13]
				fu_hdr = (FU_HEADER*)&m_SendBuf[payloadStart + 1];
				fu_hdr->R = 0;
				fu_hdr->S = 0;
				fu_hdr->TYPE = m_Mynalu_t->nal_unit_type;
				fu_hdr->E = 1;
				if (l == 0)
				{
					l = MAX_RTP_PKT_LENGTH;//刚好整除的  2800/1400=2   %0
				}
				nalu_payload = &m_SendBuf[payloadStart + 2];//同理将m_SendBuf[14]的地址赋给nalu_payload
				if (l > 0)
				{
					memcpy(nalu_payload, m_Mynalu_t->buf + t * MAX_RTP_PKT_LENGTH + 1, l );//将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入m_SendBuf[14]开始的字符串。
					bytes = l  + payloadStart + 2;		//获得m_SendBuf的长度,为剩余nalu的长度l-1加上rtp_header，FU_INDICATOR,FU_HEADER三个包头共14字节
					if (gConfig.GetCrc_() == 1)
					{
						int crc = crc_device_.crc32_table((uint8_t*)(m_SendBuf + payloadStart), (uint8_t*)(m_SendBuf + payloadStart + l + 2));
						AddPoseData* add_data = (AddPoseData*)&m_SendBuf[16];
						memmove((void*)&add_data->predictedTimeMs, &crc, sizeof(int));
					}
					int ret = sendto(m_SocketClient, m_SendBuf, bytes, 0, (sockaddr *)&m_Dst_Addr, sizeof(m_Dst_Addr));
					RtpQualityHelper::GetInstance()->SaveRtpBuf(m_SendBuf, bytes, mEyeIndex, true);
				}
				t++;
			}
			else if (t < k && 0 != t)
			{
				//设置rtp M 位；
				rtp_hdr->marker = 0;
				//设置FU INDICATOR,并将这个HEADER填入m_SendBuf[12]
				fu_ind = (FU_INDICATOR*)&m_SendBuf[payloadStart]; //将m_SendBuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入m_SendBuf中；
				fu_ind->F = m_Mynalu_t->forbidden_bit;
				fu_ind->NRI = m_Mynalu_t->nal_reference_idc >> 5;
				fu_ind->TYPE = 28;
				//设置FU HEADER,并将这个HEADER填入m_SendBuf[13]
				fu_hdr = (FU_HEADER*)&m_SendBuf[payloadStart + 1];
				//fu_hdr->E=0;
				fu_hdr->R = 0;
				fu_hdr->S = 0;
				fu_hdr->E = 0;
				if ((t == (k - 1)) && (l == 0))
				//所以在此设置end 和mark 
				{
					if (frameEnd)
					{
						rtp_hdr->marker = 1;
						m_unTimespaceCurrent = m_unTimespaceCurrent + m_unTimestampIncrese;
					}					
					fu_hdr->E = 1;

				}
				fu_hdr->TYPE = m_Mynalu_t->nal_unit_type;

				nalu_payload = &m_SendBuf[payloadStart + 2];//同理将m_SendBuf[14]的地址赋给nalu_payload
				memcpy(nalu_payload, m_Mynalu_t->buf + t * MAX_RTP_PKT_LENGTH + 1, MAX_RTP_PKT_LENGTH);//去掉起始前缀的nalu剩余内容写入m_SendBuf[14]开始的字符串。
				bytes = MAX_RTP_PKT_LENGTH + payloadStart + 2;						//获得m_SendBuf的长度,为nalu的长度（除去原NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节
				if (gConfig.GetCrc_() == 1)
				{
					int crc = crc_device_.crc32_table((uint8_t*)(m_SendBuf + payloadStart), (uint8_t*)(m_SendBuf + payloadStart + MAX_RTP_PKT_LENGTH + 2));
					AddPoseData* add_data = (AddPoseData*)&m_SendBuf[16];
					memmove((void*)&add_data->predictedTimeMs, &crc, sizeof(int));
				}
				int ret = sendto(m_SocketClient, m_SendBuf, bytes, 0, (sockaddr *)&m_Dst_Addr, sizeof(m_Dst_Addr));
				RtpQualityHelper::GetInstance()->SaveRtpBuf(m_SendBuf, bytes, mEyeIndex, true);
				t++;
				if ((t == k) && (l == 0))//已经打包完的情况t++后结束循环。
				{
					t++;
				}
			}
		}
	}
}
//NAL_UNIT_VPS,                   // 32  
	//NAL_UNIT_SPS,                   // 33  
	//NAL_UNIT_PPS,                   // 34  
	//NAL_UNIT_ACCESS_UNIT_DELIMITER, // 35  
	//NAL_UNIT_EOS,                   // 36  
	//NAL_UNIT_EOB,                   // 37  
	//NAL_UNIT_FILLER_DATA,           // 38  
	//NAL_UNIT_SEI,                   // 39 Prefix SEI  
	//16-21  I帧
void RtpPacket::DoPacketH265(uint8_t * inputFrame, int len,int RtpAdding = 0, uint8_t * rtpAddData = NULL, unsigned short addLen = 0, bool frameEnd = true)
{
	int bytes = 0;
	char* nalu_payload=NULL;
	unsigned long csrc = htonl(10);
	m_Mynalu_t->len = len;
	memmove(m_Mynalu_t->buf, inputFrame, len);
	m_Mynalu_t->forbidden_bit = 0;//  通常为 0; //1 bit
	m_Mynalu_t->nal_unit_type = ((m_Mynalu_t->buf[0]) & 0x7E) >> 1;// 6 bit

	memset(m_SendBuf, 0, MAX_BUF_LEN);//清空m_SendBuf；此时会将上次的时间戳清空，因此需要m_unTimespaceCurrent来保存上次的时间戳值
	//rtp固定包头，为12字节,该句将m_SendBuf[0]的地址赋给rtp_hdr，以后对rtp_hdr的写入操作将直接写入m_SendBuf。
	rtp_hdr = (RTP_FIXED_HEADER*)&m_SendBuf[0];
	//设置RTP HEADER，
	rtp_hdr->payload = H265;  //负载类型号，
	rtp_hdr->version = 2;  //版本号，此版本固定为2
	rtp_hdr->ssrc = csrc;    //随机指定为10，并且在本RTP会话中全局唯一
	nalu_payload = &m_SendBuf[12];
	int payloadStart = 12;//rtp头后一位。即负载开始的位置
	if (RtpAdding == 1)
	{
		rtp_hdr->extension = 1;
		payloadStart += 4;//  sizeof  RTP_PADDING_HEAD
		rtp_pad_hdr = (RTP_PADDING_HEAD*)&m_SendBuf[12];
		rtp_pad_hdr->paddingprofile = 0;
		m_RtpHeadPaddingRealLen = addLen;//n bytes
		payloadStart += m_RtpHeadPaddingRealLen;
		unsigned short rtppaddinglen = addLen / 4; //4 byte =32 bits
		rtp_pad_hdr->paddinglen = htons(rtppaddinglen);
		memmove(&m_SendBuf[16], (void*)rtpAddData, addLen);
	}

	//	当一个NALU小于1400字节的时候，采用一个单RTP包发送
	//这部分与 h264 的不同在于 ，h264 去掉 原始码流的 nal头（1byte） 。前面增加一个NALU_HEADER（1byte）
	//（实际上是根据原 nal头，生成用于打包的nal） ，用于说明类型。
	//265 直接把 nal头打入,可以不处理 （去掉 00000001 的数据 ，第一byte就是nal头
	if (m_Mynalu_t->len <= MAX_RTP_PKT_LENGTH)
	{	
		rtp_hdr->timestamp = htonl(m_unTimespaceCurrent);
		if (m_Mynalu_t->nal_unit_type == 32 || m_Mynalu_t->nal_unit_type == 33 || m_Mynalu_t->nal_unit_type == 34 || m_Mynalu_t->nal_unit_type == 39)
		{
			rtp_hdr->marker = 0;
			m_unTimespaceCurrent = m_unTimespaceCurrent;
		}
		else
		{
			if (frameEnd)
			{
				rtp_hdr->marker = 1;
				m_unTimespaceCurrent = m_unTimespaceCurrent + m_unTimestampIncrese;
			}
			
		}
		rtp_hdr->seq_no = htons(m_usSeqNum++); //序列号，每发送一个RTP包增1	
		nalu_payload = &m_SendBuf[payloadStart];
		memmove(nalu_payload, m_Mynalu_t->buf, m_Mynalu_t->len);//直接写入m_SendBuf[12]开始的字符串。		
		bytes = m_Mynalu_t->len + payloadStart;						//获得m_SendBuf的长度,为nalu的长度加上rtp_header的固定长度12字节
#ifdef THREADSEND
		memmove(mSendBufQueue[mSendBufIndex%SENDBFUSIZE], m_SendBuf, bytes);
		memmove(mAddData[mSendBufIndex%SENDBFUSIZE], rtpAddData, addLen);
		mSendBufLenQueue[mSendBufIndex%SENDBFUSIZE] = bytes;
		PostThreadMessage(mSendThreadId,MSG_SOCKET_SEND,0,0);
		mSendBufIndex++;
#endif   
#ifndef THREADSEND
		if (gConfig.GetCrc_() == 1)
		{
			int crc = crc_device_.crc32_table((uint8_t*)(m_SendBuf + payloadStart), (uint8_t*)(m_SendBuf + payloadStart + m_Mynalu_t->len));
			AddPoseData* add_data = (AddPoseData*)&m_SendBuf[16];
			memmove((void*)&add_data->predictedTimeMs, &crc, sizeof(int));
		}
		int ret = sendto(m_SocketClient, m_SendBuf, bytes, 0, (sockaddr *)&m_Dst_Addr, sizeof(m_Dst_Addr));
		RtpQualityHelper::GetInstance()->SaveRtpBuf(m_SendBuf, bytes, mEyeIndex, false);
#ifdef RTPSAVE
		fwrite(m_SendBuf, bytes, sizeof(char), pf);
#endif // RTPSAVE
	
#endif
		
	}

	else if (m_Mynalu_t->len > MAX_RTP_PKT_LENGTH)
	{

		/*
		   * create the HEVC payload header and transmit the buffer as fragmentation units (FU)
		   *
		   *    0                   1
		   *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
		   *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		   *   |F|   Type    |  LayerId  | TID |
		   *   +-------------+-----------------+
		   *
		   *      F       = 0     1bit
		   *      Type    = 49 (fragmentation unit (FU)) 6bit
		   *      LayerId = 0 6bit
		   *      TID     = 1  3bit
		   */

		   /*
			*     create the FU header
			*
			*     0 1 2 3 4 5 6 7
			*    +-+-+-+-+-+-+-+-+
			*    |S|E|  FuType   |
			*    +---------------+
			*
			*       S       = variable
			*       E       = variable
			*       FuType  = NAL unit type
			*/

			//得到该nalu需要用多少长度为1400字节的RTP包来发送
		int k = 0, l = 0;
		k = (m_Mynalu_t->len-2) / MAX_RTP_PKT_LENGTH;//需要k个1400字节的RTP包
		l = (m_Mynalu_t->len-2) % MAX_RTP_PKT_LENGTH;//最后一个RTP包的需要装载的字节数

		int t = 0;//用于指示当前发送的是第几个分片RTP包
		rtp_hdr->timestamp = htonl(m_unTimespaceCurrent);
		if (frameEnd)
		{
			m_unTimespaceCurrent = m_unTimespaceCurrent + m_unTimestampIncrese;
		}
		
		while (t <= k)
		{
			rtp_hdr->seq_no = htons(m_usSeqNum++); //序列号，每发送一个RTP包增1
			if (!t)//发送一个需要分片的NALU的第一个分片，置FU HEADER的S位
			{
				//设置rtp M 位；
				rtp_hdr->marker = 0;
				char* hdr1 = (char*)&m_SendBuf[payloadStart];
				char* hdr2 = (char*)&m_SendBuf[payloadStart+1];
				*hdr1 = 49 << 1;
				*hdr2 = 1;
				//设置FU HEADER,并将这个HEADER填入m_SendBuf[14]
				fu_265hdr = (H265_FU_HEADER*)&m_SendBuf[payloadStart+2];
				fu_265hdr->E = 0;
				fu_265hdr->S = 1;
				fu_265hdr->TYPE = m_Mynalu_t->nal_unit_type;
				nalu_payload = &m_SendBuf[payloadStart + 3];//同理将m_SendBuf[15]赋给nalu_payload
				memmove(nalu_payload, m_Mynalu_t->buf + 2, MAX_RTP_PKT_LENGTH);//去掉2 BYTES 265NALU头
				
				bytes = MAX_RTP_PKT_LENGTH + payloadStart+3;						//获得m_SendBuf的长度,为nalu的长度（除去起始前缀和NALU头）加上rtp_header，fu_ind 1 BYETS,265NALU头2BYTES 的固定长度15字节
				
#ifdef THREADSEND
				memmove(mSendBufQueue[mSendBufIndex%SENDBFUSIZE], m_SendBuf, bytes);
				memmove(mAddData[mSendBufIndex%SENDBFUSIZE], rtpAddData, addLen);
				mSendBufLenQueue[mSendBufIndex%SENDBFUSIZE] = bytes;
				PostThreadMessage(mSendThreadId, MSG_SOCKET_SEND, 0, 0);
				mSendBufIndex++;
#endif   
#ifndef THREADSEND
				if (gConfig.GetCrc_() == 1)
				{
					int crc = crc_device_.crc32_table((uint8_t*)(m_SendBuf + payloadStart), (uint8_t*)(m_SendBuf + payloadStart + MAX_RTP_PKT_LENGTH+3));
					AddPoseData* add_data = (AddPoseData*)&m_SendBuf[16];
					memmove((void*)&add_data->predictedTimeMs, &crc, sizeof(int));
				}
				int ret = sendto(m_SocketClient, m_SendBuf, bytes, 0, (sockaddr *)&m_Dst_Addr, sizeof(m_Dst_Addr));
				RtpQualityHelper::GetInstance()->SaveRtpBuf(m_SendBuf, bytes, mEyeIndex, true);
#ifdef RTPSAVE
				fwrite(m_SendBuf, bytes, sizeof(char), pf);
#endif // RTPSAVE
#endif				
				
				t++;
			}
			//发送一个需要分片的NALU的非第一个分片，清零FU HEADER的S位，如果该分片是该NALU的最后一个分片，置FU HEADER的E位
			else if (k == t)//发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节（当l>1386时）。
			{
				//设置rtp M 位；当前传输的是最后一个分片时该位置1
				if (frameEnd)
				{
					rtp_hdr->marker = 1;
				}
				//设置 hrd 到 12bytes 后
				char* hdr1 = (char*)&m_SendBuf[payloadStart];
				char* hdr2 = (char*)&m_SendBuf[payloadStart+1];
				*hdr1 = 49 << 1;  // hrd  的 type 设置为 49
				*hdr2 = 1;
				//设置FU HEADER,并将这个HEADER填入m_SendBuf[13]
				fu_265hdr = (H265_FU_HEADER*)&m_SendBuf[payloadStart+2];
				fu_265hdr->S = 0;
				fu_265hdr->TYPE = m_Mynalu_t->nal_unit_type;
				fu_265hdr->E = 1;
				if (l == 0)
				{
					l = MAX_RTP_PKT_LENGTH;//刚好整除的  2800/1400=2   %0
				}
				nalu_payload = &m_SendBuf[payloadStart+3];//同理将m_SendBuf[14]的地址赋给nalu_payload
				if ((l) > 0)
				{
					memmove(nalu_payload, m_Mynalu_t->buf + t * MAX_RTP_PKT_LENGTH + 2, l );//将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入m_SendBuf[14]开始的字符串。
					bytes = l + payloadStart+3;		//获得m_SendBuf的长度,为剩余nalu的长度l-1加上rtp_header，FU_INDICATOR,FU_HEADER三个包头共14字节
					
#ifdef THREADSEND
					memmove(mSendBufQueue[mSendBufIndex%SENDBFUSIZE], m_SendBuf, bytes);
					memmove(mAddData[mSendBufIndex%SENDBFUSIZE], rtpAddData, addLen);
					mSendBufLenQueue[mSendBufIndex%SENDBFUSIZE] = bytes;
					PostThreadMessage(mSendThreadId, MSG_SOCKET_SEND, 0, 0);
					mSendBufIndex++;
#endif   
#ifndef THREADSEND
					if (gConfig.GetCrc_() == 1)
					{
						int crc = crc_device_.crc32_table((uint8_t*)(m_SendBuf + payloadStart), (uint8_t*)(m_SendBuf + payloadStart + l + 3));
						AddPoseData* add_data = (AddPoseData*)&m_SendBuf[16];
						memmove((void*)&add_data->predictedTimeMs, &crc, sizeof(int));
					}
					int ret = sendto(m_SocketClient, m_SendBuf, bytes, 0, (sockaddr *)&m_Dst_Addr, sizeof(m_Dst_Addr));
					RtpQualityHelper::GetInstance()->SaveRtpBuf(m_SendBuf, bytes, mEyeIndex, true);
#ifdef RTPSAVE
					fwrite(m_SendBuf, bytes, sizeof(char), pf);
#endif // RTPSAVE
#endif  
				}
				t++;
				
			}
			else if (t < k && 0 != t)
			{
				//设置rtp M 位；
				rtp_hdr->marker = 0;
				char* hdr1 = (char*)&m_SendBuf[payloadStart];
				char* hdr2 = (char*)&m_SendBuf[payloadStart+1];
				*hdr1 = 49 << 1;
				*hdr2 = 1;
				//设置FU HEADER,并将这个HEADER填入m_SendBuf[14]
				fu_265hdr = (H265_FU_HEADER*)&m_SendBuf[payloadStart+2];
				fu_265hdr->E = 0;
				fu_265hdr->S = 0;
				if ((t == (k - 1)) && (l == 0))//复制时 起始位置 +2 因此 取余=2的时候刚好本帧打包完
				//所以在此设置end 和mark 
				{
					if (frameEnd)
					{
						rtp_hdr->marker = 1;
						m_unTimespaceCurrent = m_unTimespaceCurrent + m_unTimestampIncrese;
					}
					fu_265hdr->E = 1;

				}
				fu_265hdr->TYPE = m_Mynalu_t->nal_unit_type;
				nalu_payload = &m_SendBuf[payloadStart+3];//同理将m_SendBuf[15]赋给nalu_payload
				memmove(nalu_payload, m_Mynalu_t->buf + t * MAX_RTP_PKT_LENGTH + 2, MAX_RTP_PKT_LENGTH);//去掉起始前缀的265nalu剩余内容写入m_SendBuf[15]开始的字符串。
				bytes = MAX_RTP_PKT_LENGTH + payloadStart + 3;						//获得m_SendBuf的长度,为nalu的长度（除去原NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节			
#ifdef THREADSEND
				memmove(mSendBufQueue[mSendBufIndex%SENDBFUSIZE], m_SendBuf, bytes);
				memmove(mAddData[mSendBufIndex%SENDBFUSIZE], rtpAddData, addLen);
				mSendBufLenQueue[mSendBufIndex%SENDBFUSIZE] = bytes;
				PostThreadMessage(mSendThreadId, MSG_SOCKET_SEND, 0, 0);
				mSendBufIndex++;
#endif   
#ifndef THREADSEND
				if (gConfig.GetCrc_() == 1)
				{
					int crc = crc_device_.crc32_table((uint8_t*)(m_SendBuf + payloadStart), (uint8_t*)(m_SendBuf + payloadStart + MAX_RTP_PKT_LENGTH + 3));
					AddPoseData* add_data = (AddPoseData*)&m_SendBuf[16];
					memmove((void*)&add_data->predictedTimeMs, &crc, sizeof(int));
				}
				int ret = sendto(m_SocketClient, m_SendBuf, bytes, 0, (sockaddr *)&m_Dst_Addr, sizeof(m_Dst_Addr));	
				RtpQualityHelper::GetInstance()->SaveRtpBuf(m_SendBuf, bytes, mEyeIndex, true);
#ifdef RTPSAVE
				fwrite(m_SendBuf, bytes, sizeof(char), pf);
#endif // RTPSAVE
#endif 
				t++;
				if ((t == k) && (l == 0))//已经打包完的情况t++后结束循环。
				{
					t++;
				}
			}
		}
	}
	//delete[]inputFrame;
}
bool IsHead(uint8_t *buf,int i)
{
	if ((buf[i]==0)&&(buf[i+1]==0)&&(buf[i+2]==0)&&(buf[i+3]==1))
	{
		return true;
	} 
	return false;
}

bool ishead(uint8_t *buf, int i)
{
	if ((buf[i] == 0 )&& (buf[i+1] == 0) &&( buf[i+2] ==1))
	{
			
		return true;
	}
	return false;
}
int  FindStartHead(uint8_t *buf, int i,int len)
{
	if (i < len - 4)
	{
		if (IsHead(buf, i))
		{
			return 4;
		}
	}
	if (ishead(buf, i))
	{
		return 3;
	}
	return 0;	
}
#ifdef DEBUGWIRTEFILE
FILE *pf1 = fopen("testleft.h265","wb+");
#endif

void RtpPacket::DoPacket(uint8_t * inputFrames, int len, int ifH264 = 1, int RtpAdding = 0, uint8_t * rtpAddData = NULL, unsigned short addLen = 0/*actual practical data length*/)
{
	
	
	if (len==0)
	{
		return;
	}
	int i = 0;	
	int NaluBegin = 0;
	bool idr = false;
	int Interval = 0;/// i帧 h264 2+分片数。 普通 分片数 h265 3+分片数 
	std::string msg;
	
	if (ifH264 == 1) 
	{
		if (inputFrames[4])
		{
			char itype = ((inputFrames[4]) & 0x1F) ;
			if (itype == 7)
			{
				idr = true;
			}
		}
	}
	else
	{
		if (inputFrames[4])
		{
			char itype = ((inputFrames[4]) & 0x7E) >> 1;
			if (itype == 32)
			{
				idr = true;
			}
		}
	}
	
	while (1)
	{
		int ret = 0;
		if (i < len - 3)
		{
			//ret= FindStartHead(inputFrames, i, len);
			if ((inputFrames[i] == 0) && (inputFrames[i + 1] == 0) && (inputFrames[i + 2] == 0) && (inputFrames[i + 3] == 1))
			{
				ret=4;
				Interval++;
				/*if (idr)
				{
					msg = "Interva" + std::to_string(Interva);
					GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				}*/
			}
			if (gConfig.GetRtpSlice()==1)
			{
				if (ret == 0)
				{
					if ((inputFrames[i] == 0) && (inputFrames[i + 1] == 0) && (inputFrames[i + 2] == 1))
					{
						ret = 3;
						Interval++;
					}

				}
			}
		
		}		 	
		if (ret!=0)
		{
			
			if ((i - 1) >0 && NaluBegin>0)
			{
				if (ifH264==1)
				{
					DoPacketH264(inputFrames + NaluBegin, i - NaluBegin, RtpAdding, rtpAddData, addLen, false);
				}
				else  
				{					
					DoPacketH265(inputFrames+ NaluBegin,i-NaluBegin, RtpAdding, rtpAddData, addLen,false);

				}
				if (idr)
				{
					Sleep(1);
				}
#ifdef DEBUGWIRTEFILE

				fwrite(inputFrames + NaluBegin, sizeof(char), i - NaluBegin, pf1);
#endif
			}			
				NaluBegin = i + ret;
				i = NaluBegin;
#ifdef DEBUGWIRTEFILE
				if (ret==3)
				{
					char head[3] = { 0,0,1 };
					fwrite(head, sizeof(char), 3, pf1);
				} 
				else if (ret==4)
				{
					char head[4] = { 0,0,0,1 };
					fwrite(head, sizeof(char), 4, pf1);
				}					
#endif	
		}
		else
		{		
			/*if (i >= (len - 4))*/
			if ((idr==false)||(idr&&Interval>=4&&ifH264==false)|| (idr&&Interval >= 3 && ifH264  ))
			{				 

				if (ifH264 == 1)
				{
					DoPacketH264(inputFrames + NaluBegin, len - NaluBegin, RtpAdding, rtpAddData, addLen);
				}
				else
				{
					DoPacketH265(inputFrames + NaluBegin, len - NaluBegin, RtpAdding, rtpAddData, addLen);
				}
				if (idr)
				{
					Sleep(1);
				}
#ifdef DEBUGWIRTEFILE
				 
				fwrite(inputFrames + NaluBegin, sizeof(char), len - NaluBegin, pf1);
#endif
				/*msg = "bareak i=" + std::to_string(i)+"len=" + std::to_string(len);
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);*/
				break;
			}
			i++;
		}
		 
	}
}
RtpPacket::~RtpPacket()
{
}



TcpPacket* TcpPacket::gLeftTcpInstance = nullptr;

TcpPacket::GC TcpPacket::leftTcpgc;

TcpPacket* TcpPacket::gRightTcpInstance = nullptr;
TcpPacket::GC TcpPacket::rightTcpgc;


TcpPacket* TcpPacket::GetInstance(int IndexFlag)
{
	if (IndexFlag == 0)
	{
		if (gLeftTcpInstance == NULL)
		{
			return CreateInstance(0);
		}
		return gLeftTcpInstance;
	}
	else
	{
		if (gRightTcpInstance == NULL)
		{
			return CreateInstance(1);
		}
		return gRightTcpInstance;
	}
	return nullptr;
}

TcpPacket* TcpPacket::CreateInstance(int IndexFlag)
{
	if (IndexFlag == 0)
	{
		gLeftTcpInstance = new TcpPacket();
		gLeftTcpInstance->mEyeIndex = 0;
	}
	else
	{
		gRightTcpInstance = new TcpPacket();
		gRightTcpInstance->mEyeIndex = 1;
	}
	
	return nullptr;
}
 
int TcpPacket::InitSocket(std::string ip, u_short port)
{
	if (init_flag_)
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("tcp has init ");
		return 1;
	}
	HANDLE ret = (HANDLE)_beginthreadex(NULL, 0, &ReconnectThread, &mEyeIndex, 0, &mReconnectThreadId);
	m_SocketClient = socket(AF_INET, SOCK_STREAM,   0 );
	
	if (m_SocketClient == INVALID_SOCKET)
	{
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways("Socket failed，Exit!");
		return 0;
	}

	int nNetTimeout = 1000 / gConfig.GetFps() * 10;//10秒，

	//setsockopt(m_SocketClient, SOL_SOCKET, SO_SNDTIMEO, (char*)&nNetTimeout, sizeof(int));
	int sendbuf =6*1024*1024;
	//setsockopt(m_SocketClient, SOL_SOCKET, SO_SNDBUF, (char*)&sendbuf, sizeof(int));
	 
	const char chOpt = 1;
	int   nErr = setsockopt(m_SocketClient, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));
	 
    nErr = setsockopt(m_SocketClient, SOL_SOCKET, SO_KEEPALIVE, &chOpt, sizeof(char));
	struct linger so_linger;
	so_linger.l_onoff = 0;
	so_linger.l_linger = 0;
	setsockopt(m_SocketClient, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof so_linger);
	
	memset(&dst_addr_, 0, sizeof(dst_addr_));
	dst_addr_.sin_family = AF_INET;
	dst_addr_.sin_addr.S_un.S_addr = inet_addr(ip.c_str()); //旧函数，不设置sdl检查会报错
	dst_addr_.sin_port = htons(port);
	dst_port_ = port;
	init_flag_ = true;
	if (connect(m_SocketClient, (SOCKADDR*)&dst_addr_, sizeof(SOCKADDR)) == INVALID_SOCKET)
	{
		string msg = "init connect" + to_string(mEyeIndex) + "failed" + "dst:" + dst_ip_ + "port" + std::to_string(dst_port_);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
		return 0;
	}
	string msg = "connect" + to_string(mEyeIndex) + "success" + "dst:" + dst_ip_ + "port" + std::to_string(dst_port_);
	GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
	mConnectTime = nowInNs();
	m_Connected = true;
	return 1;
}

int TcpPacket::SendToServer(uint8_t* video, int len, uint8_t* pose, uint8_t ploadType ,uint8_t eyeIndex)
{
	
	if (m_Connected == false)
	{
		PostThreadMessage(mReconnectThreadId, MSG_RECONNECT, (WPARAM) & (mEyeIndex), 0);
		return 0;
	}
	uint64_t sendbegint = nowInNs();
	char* sendBuf = NULL;
	int ret = 0;
	if (eyeIndex == 0)
	{
		memmove(mSendBuf, &len, sizeof(int));
		memmove(mSendBuf + 4, &ploadType, sizeof(uint8_t));
		memmove(mSendBuf + 5, &eyeIndex, sizeof(uint8_t));
		memmove(mSendBuf + 6, pose, sizeof(AddPoseData));
		memmove(mSendBuf + 6 + sizeof(AddPoseData), video, len);
		int sendLen = len + 6 + sizeof(AddPoseData);
		//ret = MulSend(mSendBuf, sendLen, 0);
		ret = send(m_SocketClient, mSendBuf, sendLen, 0);
		 
	}
	else
	{
		memmove(mOtherSendBuf, &len , sizeof(int));
		memmove(mOtherSendBuf + 4, &ploadType, sizeof(uint8_t));
		memmove(mOtherSendBuf + 5, &eyeIndex, sizeof(uint8_t));
		memmove(mOtherSendBuf + 6, pose, sizeof(AddPoseData));
		memmove(mOtherSendBuf + 6 + sizeof(AddPoseData), video, len);
		int sendLen = len + 6 + sizeof(AddPoseData);
		//ret = MulSend(mOtherSendBuf, sendLen, 0);
		ret = send(m_SocketClient, mOtherSendBuf, sendLen, 0);
	}

	uint64_t sendendt = nowInNs();
	 
	if (((sendendt - sendbegint) > 18000000)&&((sendendt-mConnectTime)> 8000000000))
	{
		string msg = "send too big :" + to_string(double(sendendt - sendbegint) / 1000000.0f)+"len="+to_string(len+64);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
	}
	return ret;
}
int  TcpPacket::MulSend(char* buf, int len,  int eyeIndex)
{
	
	int singlelen = 1024 * 64-1;
	int usedlen = 0;
	int sendtime = 0;
	while (len>0)
	{
		 
			int sendLen = singlelen < len ? singlelen : len;
			int ret = send(m_SocketClient, (const char*)buf + usedlen, sendLen, 0);
			if (ret == SOCKET_ERROR)
			{
				return ret;  
			}
			usedlen += sendLen;
			len = len - sendLen;	
			sendtime++;
			if (sendtime%5==0)
			{
				Sleep(1);
			}
	}
	return 1;
}
void TcpPacket::SetDstIp(std::string ip) 
{
	dst_ip_ = ip;
	string msg = "tcp set_dst_ip:" + ip + "port:" + to_string(dst_port_);
	GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
	dst_addr_.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
}
unsigned int TcpPacket::ReconnectThread(LPVOID lpParameter) 
{
	int packet_index = *(int*)lpParameter;
	MSG msg;
	PeekMessage(&msg, NULL, MSG_RECONNECT, MSG_RECONNECT, PM_NOREMOVE);
	while (true)
	{
		if (GetMessage(&msg, nullptr, MSG_RECONNECT, MSG_RECONNECT)) //get msg from message queue
		{

			switch (msg.message)
			{
			case MSG_RECONNECT:
			 {
				if (TcpPacket::GetInstance(packet_index)->m_Connected==false)
				{
					TcpPacket::GetInstance(packet_index)->ReConnectServer();
				}
			 }
			}
		}
	}
	return 0;
}
void TcpPacket::DoPacket(uint8_t* inputFrames, int len, int ifH264 = 1, int RtpAdding = 0, uint8_t* rtpAddData = NULL, unsigned short addLen = 0/*actual practical data length*/, uint8_t eyeIndex )
{
	if (len == 0)
	{
		return;
	}
	int i = 0;
	int NaluBegin = 0;
	bool idr = false;
	int Interval = 0;/// i帧 h264 2+分片数。 普通 分片数 h265 3+分片数 
	std::string msg;

	if (ifH264 == 1)
	{
		if (inputFrames[4])
		{
			char itype = ((inputFrames[4]) & 0x1F);
			if (itype == 7)
			{
				idr = true;
			}
		}
	}
	else
	{
		if (inputFrames[4])
		{
			char itype = ((inputFrames[4]) & 0x7E) >> 1;
			if (itype == 32)
			{
				idr = true;
			}
		}
	}

	while (1)
	{
		int ret = 0;
		if (i < len - 3)
		{
			//ret= FindStartHead(inputFrames, i, len);
			if ((inputFrames[i] == 0) && (inputFrames[i + 1] == 0) && (inputFrames[i + 2] == 0) && (inputFrames[i + 3] == 1))
			{
				ret = 4;
				Interval++;
				/*if (idr)
				{
					msg = "Interva" + std::to_string(Interva);
					GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
				}*/
			}
			if (gConfig.GetRtpSlice() == 1)
			{
				if (ret == 0)
				{
					if ((inputFrames[i] == 0) && (inputFrames[i + 1] == 0) && (inputFrames[i + 2] == 1))
					{
						ret = 3;
						Interval++;
					}

				}
			}

		}
		if (ret != 0)
		{

			if ((i - 1) > 0 && NaluBegin > 0)
			{
				int framelen = i - NaluBegin;
				uint8_t pload = gRtpPloadTpye;
				uint8_t eye = eyeIndex;
				int ret=SendToServer(inputFrames + NaluBegin, framelen, rtpAddData, pload, eye);
				if (ret== SOCKET_ERROR)
				{
					int error_code = WSAGetLastError();
					string msg = to_string(mEyeIndex)+"SOCKET_ERROR "+to_string(error_code);
					GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
					m_Connected = false;
					PostThreadMessage(mReconnectThreadId, MSG_RECONNECT, (WPARAM) & (mEyeIndex), 0);
				}
#ifdef DEBUGWIRTEFILE

				fwrite(inputFrames + NaluBegin, sizeof(char), i - NaluBegin, pf1);
#endif
			}
			NaluBegin = i + ret;
			i = NaluBegin;
#ifdef DEBUGWIRTEFILE
			if (ret == 3)
			{
				char head[3] = { 0,0,1 };
				fwrite(head, sizeof(char), 3, pf1);
			}
			else if (ret == 4)
			{
				char head[4] = { 0,0,0,1 };
				fwrite(head, sizeof(char), 4, pf1);
			}
#endif	
		}
		else
		{
			/*if (i >= (len - 4))*/
			if ((idr == false) || (idr && Interval >= 4 && ifH264 == false) || (idr && Interval >= 3 && ifH264))
			{
				int framelen = len - NaluBegin;
				uint8_t pload = gRtpPloadTpye;
				uint8_t eye = eyeIndex;
				int ret = SendToServer(inputFrames + NaluBegin, framelen, rtpAddData, pload,eye);
				if (ret == SOCKET_ERROR)
				{
					int error_code = WSAGetLastError();
					string msg = to_string(eyeIndex)+"  SOCKET_ERROR " + to_string(error_code);
					GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
					m_Connected = false;
					PostThreadMessage(mReconnectThreadId, MSG_RECONNECT, (WPARAM)& (mEyeIndex), 0);
					//gInsertIdr =4;
				}
				//fwrite(head, sizeof(char), 4, pftcp);
				//fwrite(TcpPacket::GetInstance(mEyeIndex)->m_TcpSendBuf + 4 + sizeof(AddPoseData), sizeof(char), framelen, pftcp);
#ifdef DEBUGWIRTEFILE

				fwrite(inputFrames + NaluBegin, sizeof(char), len - NaluBegin, pf1);
#endif
				/*msg = "bareak i=" + std::to_string(i)+"len=" + std::to_string(len);
				GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);*/
				break;
			}
			i++;
		}

	}
}
int TcpPacket::ReConnectServer() 
{
	
		closesocket(m_SocketClient);
		Sleep(5);
		m_SocketClient = socket(
			AF_INET,           //internetwork: UDP, TCP, etc
			SOCK_STREAM,        //TCP
			0                  //protocol
		);

		if (m_SocketClient == INVALID_SOCKET)
		{
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways("Socket failed，Exit!");
			return 0 ;
		}
		int nNetTimeout = 1000 / gConfig.GetFps() * 10;

		//setsockopt(m_SocketClient, SOL_SOCKET, SO_SNDTIMEO, (char*)&nNetTimeout, sizeof(int));
		int sendbuf = 6 * 1024 * 1024;
		setsockopt(m_SocketClient, SOL_SOCKET, SO_SNDBUF, (char*)&sendbuf, sizeof(int));

		const char chOpt = 1;
		int   nErr = setsockopt(m_SocketClient, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));

		nErr = setsockopt(m_SocketClient, SOL_SOCKET, SO_KEEPALIVE, &chOpt, sizeof(char));
		struct linger so_linger;
		so_linger.l_onoff = 0;
		so_linger.l_linger = 0;
		setsockopt(m_SocketClient, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof so_linger);
		if (connect(m_SocketClient, (SOCKADDR*)&dst_addr_, sizeof(SOCKADDR)) == INVALID_SOCKET)
		{
			string msg = "connect:" + to_string(mEyeIndex) + " failed"+"dst:"+ dst_ip_+"port"+std::to_string(dst_port_);
			GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
			return 0;
		}
		string msg = "connect" + to_string(mEyeIndex) + "success" + "dst:" + dst_ip_ + "port" + std::to_string(dst_port_);
		GLOBAL_DLL_CONTEXT_LOG()->LogAlways(msg);
		m_Connected = true;
		mConnectTime = GetTimestampUs();
	
	return 1;
}

SendIndex::SendIndex()
{
	InitializeCriticalSection(&mCSIndex);
}

SendIndex::~SendIndex()
{
	DeleteCriticalSection(&mCSIndex);
}

void SendIndex::AddIndex() 
{
	EnterCriticalSection(&mCSIndex);
	mSendIndex++;
	LeaveCriticalSection(&mCSIndex);
}

int SendIndex::GetInedx() 
{
	int retval = 0;
	EnterCriticalSection(&mCSIndex);
	retval=mSendIndex;
	LeaveCriticalSection(&mCSIndex);
	return retval;
}