#pragma once
#include <stdint.h>
#define SIZE_OF_FEC_HEAD 8

#define FEC_DEBUG
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


//7 bytes 
typedef struct
{
	/**//* byte 0 */	 
	unsigned char buffer_num : 5;        /**//*  max 32 */
	unsigned char mark : 1;        /**//* mark  */
	unsigned char version : 2;        /**//* expect   3 (11)*/
	unsigned char unused;
	/**//* bytes 1  ,2 */
	unsigned short seq_no_begin;
	/**//* bytes 3 4 5 6 */
	unsigned int all_length;            /**//* stream number is used here. */
} FEC_HEAD;

typedef struct TimeSync_{
	uint32_t type;// 0 普通包 1 left重传请求 2 right 序号为 sequence
	uint32_t mode; // 0,1,2
	uint64_t sequence;
	uint64_t serverTime; //us
	uint64_t clientTime;  //us
	uint32_t getLost;//0,1
	uint32_t stepNumber;// 需要获取的包
	// Following value are filled by client only when mode=0.
	uint64_t packetsLostTotal;
	uint64_t packetsLostInSecond;
	uint32_t frameIndex;
	uint32_t transportLatency;
	uint32_t decodeLatency;
	uint32_t renderLatency;
	uint32_t totalLatency;
	uint32_t renderDiffer;
}TimeSync;