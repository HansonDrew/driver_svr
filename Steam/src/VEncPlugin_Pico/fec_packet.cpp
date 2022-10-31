#include <stdlib.h>
#include <windows.h>

#include "fec_packet.h"
#include "fec_define.h"
 
//#include "test_function.h"
 
void fec_packet::init(int fec_protect_num)
{
	fec_protect_num_ = fec_protect_num;
	for (int i=0;i<32;i++)
	{
		rtp_buf_[i] = new char[1500];
	}
}
bool fec_packet::save_current_frame(char* rtp, int length )
{
	RTP_FIXED_HEADER* rtp_head = (RTP_FIXED_HEADER*)rtp;
	memmove(&rtp_buf_[rtp_bufs_size_][0], rtp, length);
	rtp_buf_length_[rtp_bufs_size_] = length;
	rtp_bufs_size_++;
	if ((rtp_bufs_size_>= fec_protect_num_)||rtp_head->marker)
	{
		make_fecpacket();
		return true;//make fec packet
	}
	return false;//not make fec packet
}
void fec_packet::make_fecpacket() 
{
	fec_buf_length_ = 0;
	memset(fec_buf_,0, 1500);
	fec_buf_length_ = 0;
	FEC_HEAD* fec_head = (FEC_HEAD*)fec_buf_;
	fec_head->buffer_num = rtp_bufs_size_;
	int all_length = 0;
	int max_len = 0;
	for (int i=0;i<rtp_bufs_size_;i++)
	{
		all_length = all_length + rtp_buf_length_[i];
		RTP_FIXED_HEADER* rtp_head = (RTP_FIXED_HEADER*)rtp_buf_[i];
		if (rtp_head->marker)
		{
			fec_head->mark = true;
		}if (max_len< rtp_buf_length_[0])
		{
			max_len = rtp_buf_length_[0];
		}
		if (i==0)
		{	
			fec_head->seq_no_begin = rtp_head->seq_no;
			memmove(fec_buf_+ SIZE_OF_FEC_HEAD, rtp_buf_[0], rtp_buf_length_[0]);
		    //std::string filename= "testbuf//" + std::to_string(ntohs(rtp_head->seq_no));
		    //write_file(filename.c_str(), fec_buf_ + SIZE_OF_FEC_HEAD, rtp_buf_length_[0]);
		}
		else
		{
			for (int j=0;j< rtp_buf_length_[i];j++)
			{
				char* fec_body = (char*)(fec_buf_ + SIZE_OF_FEC_HEAD);
				*(fec_body + j )^= rtp_buf_[i][j] ;
			}
			//std::string filename = "testbuf//" + std::to_string(ntohs(rtp_head->seq_no));
			//write_file(filename.c_str(), rtp_buf_[i], rtp_buf_length_[i]);
		}
		
	}
	fec_buf_length_ = max_len + SIZE_OF_FEC_HEAD;
	fec_head->all_length = all_length;
	fec_head->version = 0x3;
	rtp_bufs_size_ = 0;
}