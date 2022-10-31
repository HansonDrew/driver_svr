#pragma once

#pragma pack(push, 1)



/*
The structure of HEVC NAL unit header
+---------------+---------------+
|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|F|   Type    |  LayerId  | TID |
+-------------+-----------------+
*/
typedef struct __NALUH265
{
    unsigned short tid:3;
    unsigned short lid:6;
    unsigned short type:6;
    unsigned short f:1;
} NALUH265, *LPNALUH265;

typedef struct __FUH265
{
    unsigned short type:6;
    unsigned short e:1;
    unsigned short s:1;
} FUH265, *LPFUH265;

#define PACK_MODE_RAW_H265      0
#define PACK_MODE_RTPPAYLOAD    1

#define TYPE_AP 48  /*Aggregation Packets (APs) 0b110000*/ /*0x60...*/
#define TYPE_FU 49  /*Fragmentation Units (FUs) 0b110001*/ /*0x62...*/
#define TYPE_PACI 50  /*PAyload Content Information (PACI) 0b110010*/ /*0x64...*/

extern const char* NALU_STARTBYTES32;
extern const char* NALU_STARTBYTES24;

class h265rtpsv
{
public:
	h265rtpsv(void);
	virtual ~h265rtpsv(void);

private:
	h265rtpsv(h265rtpsv&);
	/*h265rtpsv& operator=(h265rtpsv&);*/

protected:
	unsigned int m_maxpayloadsize;
	unsigned long m_current_ts;
	unsigned short m_last_seq;
	unsigned char m_mode;
	unsigned char m_aps_allowed;
	unsigned char reserved;

public:
	void SetMaxPayloadSize(unsigned int size);
	void SetMode(int mode/* PACK_MODE_RAW or PACK_MODE_3984 */);
	void EnableAPs(bool yesno);
	void Reset();

	/*process NALUs and pack them into rtp payloads */
	/*user should check pl after each call*/
	/*if bHavemore is set, set nalu NULL for next call*/
	bool InputNalu(const char* nalu, unsigned long nlen, bool frameend,
		char*& pl, unsigned long& plen, bool& bMark, bool& bHavemore);
	/*process incoming rtp data and output NALUs, whenever possible*/
	/*user should check nalu after each call*/
	/*if bHavemore is set, set rtp NULL for next call*/
	bool InputRtpPack(const char* rtp, unsigned long rlen, char*& nalu,	unsigned long& nlen,
		bool& bMark, unsigned long& timestamp, char& ctype, bool& bHavemore);

	static char GetNaluType( const char* nalu, unsigned long len );
	static char GetNaluTypeAndMarkerFromRtp(const char* rtp, unsigned long rlen, int& marker);

protected:
	void AddToNaluBuf(const char* data, unsigned int len, bool bNewNalu=false);
	char* m_pNaluBuf;
	char* m_pNaluBufItertor;
	unsigned int m_uNaluBufSize;
	unsigned int m_uNaluDataLen;
	bool m_bFrameEndInNaluBuf;

protected:
	void AddToRtpBuf(const char* data, unsigned int len, bool bNewRtp=false);
	char* m_pRtpBuf;
	char* m_pRtpBufItertor;
	unsigned int m_uRtpBufSize;
	unsigned int m_uRtpDataLen;
	unsigned int m_uNaluCountInRtpBuf;
    //unsigned char m_cLowestLayerId;
    //unsigned char m_cLowestTId;
};

#pragma pack(pop)