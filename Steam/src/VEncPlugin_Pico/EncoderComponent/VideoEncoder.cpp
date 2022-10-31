#include "VideoEncoder.h"

void VideoEncoder::CreateOutFrameBufferAndInitParts()
{
	mEncoderRun = true;
	
	out_frame_index_ = 0;
	for (int i = 0; i < OUTBUFSIZE; i++)
	{
		mOutFrame[i].buf = new uint8_t[3 * 1024 * 1024];
		if (mOutFrame[i].buf==NULL)
		{
			//OutputDebugString(L"pico mOutFrame null");
		}
		 
		mOutFrame[i].len = 0;
	}
}

void VideoEncoder::DeleteOutFrameBuffer() 
{
	for (int i = 0; i < OUTBUFSIZE; i++)
	{
		delete[]  mOutFrame[i].buf;
	}
}