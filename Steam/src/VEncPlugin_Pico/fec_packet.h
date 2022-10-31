#pragma once
class fec_packet
{
public:
	//
	void init(int fec_protect_num = 8);
	
	bool save_current_frame(char* rtp, int length );
	
	char * rtp_buf_[32];
	int rtp_buf_length_[32] = { 0 };
	int rtp_bufs_size_ = 0;
	char fec_buf_[1500];
	int fec_buf_length_=0;
	void set_fec_protect_num(int fec_protect_num) { fec_protect_num_ = fec_protect_num; };
	int fec_protect_num_ = 8;
private:
	void make_fecpacket();
};

