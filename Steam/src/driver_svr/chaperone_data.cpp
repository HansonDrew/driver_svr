#include <string.h>
#include "chaperone_data.h"
#include "driverlog.h"
#include "hid_module.h"
#include "driverlog.h"
#include "../RVRPlugin/RVRPluginDefinitions.h"
/// <summary>
/// data/misc/user/0/boundary/stdata.txt
/// </summary>
ChaperOneData* ChaperOneData::ChaperOneDataInstance;
ChaperOneData::GC ChaperOneData::ChaperOneDataGc;

ChaperOneData* ChaperOneData::GetInstance()
{
	if (ChaperOneDataInstance == NULL)
	{
		ChaperOneDataInstance = new ChaperOneData();
	}
	return ChaperOneDataInstance;
}
ChaperOneData::ChaperOneData()
{
	data_buf_ = new RVR::RVRVector2[MAXSIZE];
}

ChaperOneData::~ChaperOneData()
{
	delete[]data_buf_;
	data_buf_ = nullptr;
}
bool ChaperOneData::OperateData(unsigned char* buf, int len)
{
	
	HidType::HidResponseData response_data = { 0 };
	memmove((void*)&response_data,buf,len);
	if (response_data.devState.type!=4)
	{
		return false;
	}
	DriverLog("save chaperone");
	data_buf_szie_ = (int)response_data.data[0] * 256 + (int)response_data.data[1];
	int begin_index= (int)response_data.data[2] * 256 + (int)response_data.data[3];
	int this_buf_len = 7;
	if ((begin_index+ this_buf_len)> data_buf_szie_)
	{
		this_buf_len = data_buf_szie_ - begin_index;
	}
	DriverLog("chaperone alllen=%d, this_len=%d, bufindex=%d", data_buf_szie_,this_buf_len, begin_index);
	RVR::RVRVector2 data[8] = { 0 };
	for (int i=0;i< this_buf_len;i++)
	{
		memmove((void*)&data[i].x, (response_data.data+4+i*sizeof(float)*2),sizeof(float));
		memmove((void*)&data[i].y, (response_data.data + 4 + i * sizeof(float) * 2+sizeof(float)), sizeof(float));
		DriverLog("chaperone data  %f  ,%f", data[i].x, data[i].y);
	}
	SaveChaperOneData(data, this_buf_len, begin_index);
	return true;
}
void ChaperOneData::SaveChaperOneData(RVR::RVRVector2* data, int data_len, int begin_inedx)
{
	memmove(data_buf_+begin_inedx,data,sizeof(RVR::RVRVector2)*data_len);
	for (int i=0;i<data_len;i++)
	{
		data_flag_[begin_inedx+i]=true;
	}
	 
}
#include <fstream>

bool ChaperOneData::CheckGetAllData() 
{
	for (int i=0;i<data_buf_szie_;i++)
	{
		if (data_flag_[i]==false)
		{
			return false;
		}
	}
	if (chaper_one_test_)
	{
		std::ofstream out("chaper_get.txt", std::ios::trunc | std::ios::out);
		if (out.fail() != false) 
		{
			for (int i = 0; i < data_buf_szie_; i++)
			{
				std::string text = std::to_string(data_buf_[i].x) + "," + std::to_string(data_buf_[i].y);
				out << text << "\n";
			}
			out.close();
		}
		
		chaper_one_test_ = false;
	}
	
	get_all_data_ = true;
	return true;
}

void ChaperOneData::ResetDataBuf() 
{
	for (int i = 0; i < MAXSIZE; i++)
	{
		data_flag_[i] = false;
	}
	data_buf_szie_ = 0;
	get_all_data_ = false;
}

int ChaperOneData::RequestChaperOneData(HidType::ChaperOneRequestType request_flag, bool test_flag)
{
	HidType::HidRequestData hid_request = { 0 };
	hid_request.request_type = (int)request_flag;
	chaper_one_test_ = test_flag;
	return HidModule::GetInstance()->WriteData((unsigned char*)&hid_request, sizeof(HidType::HidRequestData));
}
int ChaperOneData::SetChaperOne() 
{
	if (safe_region_manger==nullptr)
	{
		safe_region_manger = new saferegion_mamger();
	}
	for (int i = 0; i < data_buf_szie_; i++)
	{
		std::string text ="set chaperone index:"+std::to_string(i)+"   data: " + std::to_string(data_buf_[i].x) + "," + std::to_string(data_buf_[i].y);
		DriverLog(text.c_str());
	}
	DriverLog("call chaperone saferegion data_buf_size=%d", data_buf_szie_);
	if (data_buf_szie_>=0)
	{
		safe_region_manger->ChangeChaperOneSafeRegion(data_buf_, data_buf_szie_);
	}
	return 1;
}