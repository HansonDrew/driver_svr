#pragma once
#define  MAXSIZE 5000
#include"util.h"
#include "saferegion_manger.h"
#include "../RVRPlugin/RVRPluginDefinitions.h"
#include "driver_define.h"
class ChaperOneData
{
public:
	
	bool OperateData(unsigned char* buf, int len);
	void SaveChaperOneData(RVR::RVRVector2* data, int data_len,int begin_inedx);
	bool CheckGetAllData();
	bool GetAllDate() { return get_all_data_; };
	void ResetDataBuf();
	void SetDataBufSize(int data_buf_size) { data_buf_szie_ = data_buf_size; };
	RVR::RVRVector2* GetDataBuf() { return data_buf_; };
	int RequestChaperOneData(HidType::ChaperOneRequestType request_flag,bool test_flag=false);
	class GC  
	{
	public:
		GC()
		{
		}
		~GC()
		{
			// We can destory all the resouce here, eg:db connector, file handle and so on
			if (ChaperOneDataInstance != NULL)
			{
				delete ChaperOneDataInstance;
				ChaperOneDataInstance = NULL;
			}
		}
	};
	static GC ChaperOneDataGc;  
	static ChaperOneData* GetInstance();
	int  SetChaperOne();
private:
	saferegion_mamger* safe_region_manger = nullptr;
	bool chaper_one_test_ = false;
	ChaperOneData();
	~ChaperOneData();
	static ChaperOneData* ChaperOneDataInstance;
	RVR::RVRVector2* data_buf_;
	bool data_flag_[MAXSIZE] = { 0 };
	int data_buf_szie_ = 0;
	bool get_all_data_ = false;
};

