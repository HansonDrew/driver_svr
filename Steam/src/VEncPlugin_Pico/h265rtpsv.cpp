
#include "h265rtpsv.h"
#include <winsock2.h>
#include <stdio.h>
#pragma comment(lib, "ws2_32.lib")

h265rtpsv::h265rtpsv(void)
{
	m_maxpayloadsize = 1440;
	m_mode = PACK_MODE_RTPPAYLOAD;
	m_current_ts = 0;/*or some random value*/
	m_last_seq = 0;
	m_aps_allowed = FALSE;

	m_pNaluBuf = NULL;
	m_pNaluBufItertor = NULL;
	m_uNaluBufSize = 0;
	m_uNaluDataLen = 0;
	m_bFrameEndInNaluBuf = FALSE;

	m_pRtpBuf = NULL;
	m_pRtpBufItertor = NULL;
	m_uRtpBufSize = 0;
	m_uRtpDataLen = 0;
	m_uNaluCountInRtpBuf = 0;
    //m_cLowestLayerId = 0;
    //m_cLowestTId = 0;
}

h265rtpsv::~h265rtpsv(void)
{
	if (m_pNaluBuf)
	{
		delete [] m_pNaluBuf;
		m_pNaluBuf = NULL;
	}
	if (m_pRtpBuf)
	{
		delete [] m_pRtpBuf;
		m_pRtpBuf = NULL;
	}
}

//h265rtpsv & h265rtpsv::operator=(h265rtpsv &)
//{
//	// TODO: 在此处插入 return 语句
//}

void h265rtpsv::SetMaxPayloadSize( unsigned int size )
{
	m_maxpayloadsize = size;
}

void h265rtpsv::SetMode( int mode/* PACK_MODE_RAW or PACK_MODE_3984 */ )
{
	m_mode = mode;
}

void h265rtpsv::EnableAPs( bool yesno )
{
	m_aps_allowed = yesno;
}

void h265rtpsv::Reset()
{
	
	m_current_ts = 0;
	m_last_seq = 0;

	m_uNaluDataLen = 0;
	m_uRtpDataLen = 0;

	m_bFrameEndInNaluBuf = FALSE;

	m_pNaluBufItertor = NULL;
	m_pRtpBufItertor = NULL;

    m_uNaluCountInRtpBuf = 0;
    //m_cLowestLayerId = 0;
    //m_cLowestTId = 0;
}

/*process NALUs and pack them into rtp payloads */
bool h265rtpsv::InputNalu(const char* nalu, unsigned long nlen, bool frameend,
	char*& pl, unsigned long& plen, bool& bMark, bool& bHavemore)
{
	pl = NULL;
	plen = 0;
	bMark = FALSE;
	bHavemore = FALSE;

	switch(m_mode)
	{
	case PACK_MODE_RAW_H265:
		{ // do nothing but a check
			if (!nalu)
				return FALSE;

			char* raw = (char*)nalu;
			unsigned long rawlen = nlen;

			if (rawlen > 4)
			{
				if (memcmp(raw, NALU_STARTBYTES32, 4) == 0)
				{
					raw += 4;
					rawlen -= 4;
				}
				else if (memcmp(raw, NALU_STARTBYTES24, 3) == 0)
				{
					raw += 3;
					rawlen -= 3;
				}
			}

			if (nlen > m_maxpayloadsize)
			{
				
				return FALSE;
			}

			pl = raw;
			plen = rawlen;
			bMark = frameend;
			bHavemore = FALSE;
			return TRUE;
		}
		break;
	case PACK_MODE_RTPPAYLOAD:
		{ // do pack
			char* raw = (char*)nalu;
			unsigned long rawlen = nlen;

			if (!raw)
			{
				raw = m_pNaluBuf;
				rawlen = m_uNaluDataLen;
				if (!raw)
				{
					return FALSE;
				}
			}
			else
			{
				m_uNaluDataLen = 0;
				m_pNaluBufItertor = NULL;
			}

			if (rawlen > 4)
			{
				if (memcmp(raw, NALU_STARTBYTES32, 4) == 0)
				{
					raw += 4;
					rawlen -= 4;
				}
				else if (memcmp(raw, NALU_STARTBYTES24, 3) == 0)
				{
					raw += 3;
					rawlen -= 3;
				}
			}

			if (m_aps_allowed)
			{
				if (m_uRtpDataLen > 0)
				{
					if (m_uRtpDataLen+rawlen+2 < m_maxpayloadsize)
					{
						unsigned short size = htons((unsigned short)rawlen);
						AddToRtpBuf((char*)&size, 2);
						AddToRtpBuf(raw, rawlen);
                        if (m_pRtpBuf)
                        { // 修正聚合包头
                            unsigned short usNaluH_ap = ntohs(*(unsigned short*)m_pRtpBuf);
                            LPNALUH265 naluh_ap = (LPNALUH265)&usNaluH_ap;

                            unsigned short usNaluH = ntohs(*(unsigned short*)raw);
                            LPNALUH265 naluh = (LPNALUH265)&usNaluH;

                            if (naluh->lid < naluh_ap->lid)
                                naluh_ap->lid = naluh->lid;
                            if (naluh->tid < naluh_ap->tid)
                                naluh_ap->tid = naluh->tid;

                            unsigned short usNaluH_ap_n = htons(usNaluH_ap);
                            *(unsigned short*)m_pRtpBuf = usNaluH_ap_n;
                        }
                        m_uNaluCountInRtpBuf++;

						if (frameend)
						{
							pl = m_pRtpBuf;
							plen = m_uRtpDataLen;
							bMark = frameend;
							bHavemore = FALSE;

							m_uRtpDataLen = 0;
                            m_uNaluCountInRtpBuf = 0;
                            //m_cLowestLayerId = 0;
                            //m_cLowestTId = 0;
						}

						return TRUE;
					}
					else
					{
						m_uNaluDataLen = 0;
						m_pNaluBufItertor = NULL;
						AddToNaluBuf(raw, rawlen);

						pl = m_pRtpBuf;
						plen = m_uRtpDataLen;
						if (m_uNaluCountInRtpBuf == 1)
						{
							pl += 4;
							plen -= 4;
							//TRACE("Sending previous msg as single NAL\n");
						}
						else
						{
							//TRACE("Sending AP\n");
						}
						bMark = FALSE;
						bHavemore = TRUE;

						m_uRtpDataLen = 0;
                        m_uNaluCountInRtpBuf = 0;
                        //m_cLowestLayerId = 0;
                        //m_cLowestTId = 0;

						return TRUE;
					}
				}
				else if (rawlen<m_maxpayloadsize/2 && !frameend)
				{
					unsigned short size = htons((unsigned short)rawlen);
					AddToRtpBuf((char*)&size, 2, TRUE);
                    AddToRtpBuf(raw, rawlen);
                    if (m_pRtpBuf)
                    { // 修正聚合包头
                        unsigned short usNaluH_ap = ntohs(*(unsigned short*)m_pRtpBuf);
                        LPNALUH265 naluh_ap = (LPNALUH265)&usNaluH_ap;

                        unsigned short usNaluH = ntohs(*(unsigned short*)raw);
                        LPNALUH265 naluh = (LPNALUH265)&usNaluH;

                        naluh_ap->f = 0;
                        naluh_ap->type = TYPE_AP;
                        naluh_ap->lid = naluh->lid;
                        naluh_ap->tid = naluh->tid;

                        unsigned short usNaluH_ap_n = htons(usNaluH_ap);
                        *(unsigned short*)m_pRtpBuf = usNaluH_ap_n;
                    }
					m_uNaluCountInRtpBuf++;
					return TRUE;
				}
			}

			if (rawlen > m_maxpayloadsize)
			{
				bool bStart = FALSE;
				bool bEnd = FALSE;
				char* p = NULL;
				unsigned int l = 0;
// 				TRACE("Sending FU packets\n");
				if (m_pNaluBufItertor)
				{
					p = m_pNaluBufItertor;
				}
				else
				{
					m_uNaluDataLen = 0;
					m_pNaluBufItertor = NULL;
					AddToNaluBuf(raw, rawlen);
					p = m_pNaluBuf+2;
					bStart = TRUE;
				}

				if (m_maxpayloadsize-3 <= (unsigned int)(m_pNaluBuf+m_uNaluDataLen-p))
				{
					l = m_maxpayloadsize-3;
					bHavemore = TRUE;
				}
				else
				{
					l = m_pNaluBuf+m_uNaluDataLen-p;
					bEnd = TRUE;
					bHavemore = FALSE;
				}

                unsigned short usNaluH = ntohs(*(unsigned short*)raw);
                LPNALUH265 naluh = (LPNALUH265)&usNaluH;
                char cFuH = 0;
                LPFUH265 fuh = (LPFUH265)&cFuH;
                fuh->s = bStart;
                fuh->e = bEnd;
                fuh->type = naluh->type;
                naluh->type = TYPE_FU;
                unsigned short usNaluH_n = htons(usNaluH);

				m_uRtpDataLen = 0;
				m_pRtpBufItertor = NULL;
                AddToRtpBuf((char*)&usNaluH_n, 2);
                AddToRtpBuf(&cFuH, 1);
				AddToRtpBuf(p, l);

				p += l;

				pl = m_pRtpBuf;
				plen = m_uRtpDataLen;

				if (bHavemore)
				{
					m_pNaluBufItertor = p;
					bMark = FALSE;
				}
				else
				{
					m_pNaluBufItertor = NULL;
					bMark = frameend;
				}
				m_uRtpDataLen = 0;
				m_uNaluCountInRtpBuf = 0;

				return TRUE;
			}
			else
			{
// 				TRACE("Sending Single NAL\n");

				pl = raw;
				plen = rawlen;
				bMark = frameend;
				bHavemore = FALSE;

				return TRUE;
			}
		}
		break;
	default:
		printf("Bad or unsupported mode %i\n",m_mode);
	}
	return FALSE;
}

bool h265rtpsv::InputRtpPack( const char* rtp, unsigned long rlen, char*& nalu, unsigned long& nflen,
							 bool& bMark, unsigned long& timestamp, char& ctype, bool& bHavemore )
{
//	nalu = NULL;
//	nflen = 0;
//	bMark = FALSE;
//	timestamp = 0;
//	ctype = FALSE;
//	bHavemore = FALSE;
//
//	const char* rtpdata = rtp;
//	unsigned long rtplen = rlen;
//	if (!rtpdata)
//	{
//		rtpdata = m_pRtpBuf;
//		rtplen = m_uRtpDataLen;
//		if (!rtpdata || m_uRtpDataLen==0)
//		{
//			return FALSE;
//		}
//	}
//	else
//	{
//		m_uRtpDataLen = 0;
//		m_pRtpBufItertor = NULL;
//	}
//
//	LPRTP_HEADER h = (LPRTP_HEADER)rtpdata;
//	if (rtp)
//	{
//		unsigned short sn = ntohs(h->seq_number);
//		if (((m_last_seq+1)&0xffff) != sn)
//		{
//			TRACE("RTP packet lost sn = %d, lastsn = %d\n", sn, m_last_seq);
//			Reset();
//		}
//		m_last_seq = sn;
//	}
//// 	TRACE("h265rtpsv::InputRtpPack v=%d, p=%d, e=%d, c=%d, m=%d, pl=%d, seq=%u, ts=%u, ssrc=%u\n",
//// 		h->version, h->padbit, h->extbit, h->cc, h->markbit, h->paytype, h->seq_number, h->timestamp, h->ssrc);
//
//	char* payload = (char*)(rtpdata+12+h->cc*4+h->extbit*4);
//
//	if (h->extbit)
//	{
//		unsigned short* ext_buf = (unsigned short*)(payload-2);
//		payload += (ntohs(ext_buf[0])*4);
//	}
//
//	if (payload >= rtpdata+rtplen)
//	{
//		Reset();
//		return FALSE;
//	}
//
//    unsigned short usNaluH = ntohs(*(unsigned short*)payload);
//	LPNALUH265 naluh = (LPNALUH265)&usNaluH;
//	unsigned char type = naluh->type;
//	char* p = NULL;
//	int marker = h->markbit;
//	unsigned long ts = ntohl(h->timestamp);
//
//	if (m_current_ts!=ts && m_uNaluDataLen>0)
//	{
//		/*a new frame is arriving, in case the marker bit was not set in previous frame, output it now*/
//		m_current_ts = ts;
//		m_uRtpDataLen = 0;
//		m_pRtpBufItertor = NULL;
//		AddToRtpBuf(rtpdata, rtplen);
//		nalu = m_pNaluBuf;
//		nflen = m_uNaluDataLen;
//		bMark = 1;
//		timestamp = ts;
//		ctype = GetNaluType(nalu, nflen);
//		bHavemore = TRUE;
//
//		m_uNaluDataLen = 0;
//
//		return TRUE;
//	}
//
//	if (type == TYPE_AP)
//	{
///*
//		TRACE("Parsing TYPE_AP\n");
//*/
//		/*split into nalus*/
//		m_current_ts = ts;
//		if (m_pRtpBufItertor)
//		{
//			if (m_pRtpBufItertor<=payload || m_pRtpBufItertor>=m_pRtpBuf+m_uRtpDataLen)
//			{
//				Reset();
//				return FALSE;
//			}
//			p = m_pRtpBufItertor;
//		}
//		else
//		{
//			m_uRtpDataLen = 0;
//			m_pRtpBufItertor = NULL;
//			AddToRtpBuf(rtpdata, rtplen);
//
//			p = (char*)(m_pRtpBuf+(payload-rtpdata)+sizeof(NALUH265));
//		}
//        unsigned short sz = ntohs(*(unsigned short*)p);;
//		p += 2;
//		if (p+sz > rtpdata+rtplen)
//		{
//			Reset();
//			return FALSE;
//		}
//		AddToNaluBuf(p, sz, TRUE);
//
//		m_pRtpBufItertor = p+sz;
//		if (m_pRtpBufItertor >= m_pRtpBuf+m_uRtpDataLen)
//		{
//			m_uRtpDataLen = 0;
//			m_pRtpBufItertor = NULL;
//			bHavemore = FALSE;
//		}
//		else
//		{
//			bHavemore = TRUE;
//		}
//
//		nalu = m_pNaluBuf;
//		nflen = m_uNaluDataLen;
//		bMark = (marker&&!bHavemore);
//		timestamp = ts;
//		ctype = GetNaluType(nalu, nflen);
//
//		m_uNaluDataLen = 0;
//
//		return TRUE;
//	}
//	else if (type == TYPE_FU)
//	{
///*
//		TRACE("Parsing FU\n");
//*/
//		LPFUH265 fuh = (LPFUH265)(payload+sizeof(NALUH265));
//		if (fuh->s)
//		{
//			m_current_ts = ts;
//			if (m_uNaluDataLen != 0)
//			{
//				//TRACE("receiving FU start while previous FU-A is not finished\n");
//			}
//
//            unsigned short usNewNaluH = usNaluH;// ntohs(*(unsigned short*)payload);
//            LPNALUH265 newnaluh = (LPNALUH265)&usNewNaluH;
//            newnaluh->type = fuh->type;
//            usNewNaluH = htons(usNewNaluH);
//			AddToNaluBuf((const char*)&usNewNaluH, sizeof(usNewNaluH), TRUE);
//			AddToNaluBuf(payload+3, rtpdata+rtplen-(payload+3));
//		}
//		else
//		{
//			if (m_uNaluDataLen > 0)
//			{
//				AddToNaluBuf(payload+3, rtpdata+rtplen-(payload+3));
//			}
//			else
//			{
////				TRACE("Receiving continuation FU packet but no start.\n");
//				Reset();
//				return FALSE;
//			}
//		}
//
//		if (fuh->e && m_uNaluDataLen>0)
//		{
//			nalu = m_pNaluBuf;
//			nflen = m_uNaluDataLen;
//			bMark = marker;
//			timestamp = ts;
//			ctype = GetNaluType(nalu, nflen);
//			bHavemore = FALSE;
//
//			m_uNaluDataLen = 0;
//		}
//		return TRUE;
//    }
//    else if (type == TYPE_PACI)
//    { // 暂不支持 PACI 李招华 2014-9-1
//        return TRUE;
//    }
//	else
//	{
//		/*single nal unit*/
//// 		TRACE("Parsing single NAL\n");
//		m_current_ts = ts;
//		AddToNaluBuf(payload, rtpdata+rtplen-payload, TRUE);
//
//		nalu = m_pNaluBuf;
//		nflen = m_uNaluDataLen;
//		bMark = marker;
//		timestamp = ts;
//		ctype = GetNaluType(nalu, nflen);
//		bHavemore = FALSE;
//
//		m_uNaluDataLen = 0;
//
//		return TRUE;
//	}
//
//	if (marker)
//	{
////		TRACE("Marker bit set\n");
//		/*end of frame, output everything*/
//
//		if (m_uNaluDataLen > 0)
//		{
//			nalu = m_pNaluBuf;
//			nflen = m_uNaluDataLen;
//			bMark = marker;
//			timestamp = ts;
//			ctype = GetNaluType(nalu, nflen);
//			bHavemore = FALSE;
//
//			m_uNaluDataLen = 0;
//		}
//	}
//
	return TRUE;
}

void h265rtpsv::AddToNaluBuf( const char* data, unsigned int len, bool bNewNalu/*=FALSE*/ )
{
	if(bNewNalu)
	{
		m_uNaluDataLen = 0;
		if (memcmp(data, NALU_STARTBYTES32, 4) != 0
			&& memcmp(data, NALU_STARTBYTES24, 3) != 0)
		{
			AddToNaluBuf(NALU_STARTBYTES32, 4);
		}
	}
	
	while (m_uNaluDataLen+len > m_uNaluBufSize)
	{
		if(0 == m_uNaluBufSize)
			m_uNaluBufSize = 128*1024;
		else
			m_uNaluBufSize *= 2;

		char * pTemp = m_pNaluBuf;
		m_pNaluBuf = new char[m_uNaluBufSize];

		if (NULL != pTemp)
		{
			memcpy(m_pNaluBuf, pTemp, m_uNaluDataLen);
			delete [] pTemp;
			pTemp = NULL;
		}
	}

	memcpy(m_pNaluBuf+m_uNaluDataLen, data, len);
	m_uNaluDataLen += len;
}

void h265rtpsv::AddToRtpBuf( const char* data, unsigned int len, bool bNewRtp/*=FALSE*/ )
{
	if (bNewRtp)
    {
        m_uRtpDataLen = 0;

        // 聚合包头
        unsigned short usNaluH = 0;
        LPNALUH265 naluh = (LPNALUH265)&usNaluH;

        naluh->f = 0;
        naluh->type = TYPE_AP;
        naluh->lid = 0;
        naluh->tid = 0;

        unsigned short usNaluH_n = htons(usNaluH);
        AddToRtpBuf((char*)&usNaluH_n, 2);
    }

	while (m_uRtpDataLen+len > m_uRtpBufSize)
	{
		if(0 == m_uRtpBufSize)
			m_uRtpBufSize = 2*1024;
		else
			m_uRtpBufSize *= 2;

		char * pTemp = m_pRtpBuf;
		m_pRtpBuf = new char[m_uRtpBufSize];

		if (NULL != pTemp)
		{
			memcpy(m_pRtpBuf, pTemp, m_uRtpDataLen);
			delete [] pTemp;
			pTemp = NULL;
		}
	}

	memcpy(m_pRtpBuf+m_uRtpDataLen, data, len);
	m_uRtpDataLen += len;
}

char h265rtpsv::GetNaluType( const char* nalu, unsigned long len )
{
	if (len < 6)
	{
		return 0;
    }

    unsigned short usNaluH = 0;
	if (memcmp(nalu, NALU_STARTBYTES32, 4) == 0)
	{
        usNaluH = ntohs(*(unsigned short*)(nalu+4));
	}
	else if (memcmp(nalu, NALU_STARTBYTES24, 3) == 0)
    {
        usNaluH = ntohs(*(unsigned short*)(nalu+3));
	}
	else
	{
		return 0;
    }
    LPNALUH265 naluh = (LPNALUH265)&usNaluH;
	return naluh->type;
}

char h265rtpsv::GetNaluTypeAndMarkerFromRtp( const char* rtp, unsigned long rlen, int& marker )
{
    //marker = 0;

    //if (rlen < sizeof(RTP_HEADER))
    //    return 0;

    //LPRTP_HEADER h = (LPRTP_HEADER)rtp;
    //char* payload = (char*)(rtp+12+h->cc*4+h->extbit*4);
    //if (payload+4 > rtp+rlen)
    //    return 0;
    //if (h->extbit)
    //{
    //    unsigned short* ext_buf = (unsigned short*)(payload-2);
    //    payload += (ntohs(ext_buf[0])*4);
    //    if (payload+4 > rtp+rlen)
    //        return 0;
    //}

    //marker = h->markbit;

    //unsigned short usNaluH = ntohs(*(unsigned short*)payload);
    //LPNALUH265 naluh = (LPNALUH265)&usNaluH;
    //unsigned char type = naluh->type;

    //if (type == TYPE_AP)
    //{
    //    usNaluH = ntohs(*(unsigned short*)(payload+4/*PF HDR (2B), NALU 1 Size(2B)*/));
    //    return naluh->type;
    //}
    //else if (type == TYPE_FU)
    //{
    //    LPFUH fuh = (LPFUH)(payload+2/*FU indicator*/);
    //    return fuh->type;
    //}
    //else
    //{
    //    return naluh->type;
    //}

    return 0;
}