#ifndef NO_SLARDAR
#include<windows.h>
#include "SlardarMoudle.h"
#include "TimeTool.h"
#include "filetool.h"
#include "driverlog.h"

extern bool gLog;
using namespace parfait;
SlardarMoudle* SlardarMoudle::slardar_instance_ = nullptr;
SlardarMoudle* SlardarMoudle::GetInstance()
{
	 
		if (slardar_instance_ == nullptr)
		{
			slardar_instance_ = new SlardarMoudle();
			return slardar_instance_;
		}
		return slardar_instance_;
}
int SlardarMoudle::Init()
{
#define INSTANCE_NAME "streaming_driver" 
#define ROOT_PATH_NAME "C:\\Users\\"

	parfait_wrapper_ = CreateParfaitWrapper();

	//是否debug，开启后parfait会输出内部调试日志，默认false
	//parfait::ParfaitGlobalEnvBuilderBase::SetIsDebug(true);
	//创建全局环境变量builder
	parfait::ParfaitGlobalEnvBuilderBase* global_env_builder = CreateParfaitGlobalEnvBuilder(PICOAPPID);
	//设置全局环境参数值&初始化全局环境变量
	string device_id = GetMac();
	DriverLog("slardar deviceid=%s", device_id.c_str());
	global_env_builder
		->SetIsOverseas(false) //是否是海外业务，默认false
		.SetRootPathName(ROOT_PATH_NAME)
		.SetDid(device_id.c_str())
		.SetUid(device_id.c_str())
		.SetAppVersion("8.0.6")
		.SetChannel("pico_streaming")
		.SetReportInterval(1000)
		.SetMaxFileSize(1024*32); //非必须
	parfait_wrapper_->InitGlobalEnv(*global_env_builder);
	DestroyParfaitGlobalEnvBuilder(global_env_builder); //参数已注入，销毁全局环境变量builder

	//创建实例环境变量builder
	parfait::ParfaitEnvBuilderBase* env_builder = CreateParfaitEnvBuilder(PICOAPPID, INSTANCE_NAME);
	//设置实例环境参数值&初始化实例环境变量
	env_builder->SetProcessName("steamvr");
	parfait_wrapper_->InitInstanceEnv(*env_builder);
	DestroyParfaitEnvBuilder(env_builder); //参数已注入，销毁实例环境变量builder
	recorder_ = parfait_wrapper_->CreateUserDefinedRecorder(INTERVAL_1, "streaming");
	parfait_wrapper_->Upload();
	return 1;
}
void SlardarMoudle::ClearUp() 
{
	parfait_wrapper_->DestroyUserDefinedRecorder(recorder_);
}
/*
char* streaming_data = "{\n"
		"\t\t\"task_id\":001,\n"
		"\t\t\"streaming_data\":[[594,0.1,10,0.2,40,50],[596,0.1,10,0.2,40,50]],\n"
		"\t\t\"timestamp\":123,\n"
		"\t}";
*/
int SlardarMoudle::SendToSlardar() 
{
	if (message_buf_.size()<=0||task_id_.compare("streaming_1234")==0)
	{
		return 0;
	}
	std::string streaming_data = "{\n\t\t\"streaming_data\":[";
	buf_mutex_.lock();
	for (int i=0;i<message_buf_.size();i++)
	{
		if (i!=0)
		{
			streaming_data += ",";
		}
		streaming_data = streaming_data+"\n\t\t["+ task_id_+","+message_buf_[i]+"]";
	}
	message_buf_.clear();
	buf_mutex_.unlock();
	streaming_data += "]\n}";
	if (gLog&& (0x8000 & GetAsyncKeyState('S')) != 0)
	{
		DriverLog("slardar message  %s", streaming_data.c_str());
	}
	recorder_->WriteJsonData(streaming_data.c_str()).DoRecord();
	return 1; 
}

void SlardarMoudle::SaveMessage(std::string message)
{
	
	int64_t timestamp = GetTimestampUs();
	message = message+"," + to_string(timestamp);
	buf_mutex_.lock();
	message_buf_.push_back(message);
	buf_mutex_.unlock();
}
#endif
 