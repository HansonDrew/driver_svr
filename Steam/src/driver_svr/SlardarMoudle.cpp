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

	//�Ƿ�debug��������parfait������ڲ�������־��Ĭ��false
	//parfait::ParfaitGlobalEnvBuilderBase::SetIsDebug(true);
	//����ȫ�ֻ�������builder
	parfait::ParfaitGlobalEnvBuilderBase* global_env_builder = CreateParfaitGlobalEnvBuilder(PICOAPPID);
	//����ȫ�ֻ�������ֵ&��ʼ��ȫ�ֻ�������
	string device_id = GetMac();
	DriverLog("slardar deviceid=%s", device_id.c_str());
	global_env_builder
		->SetIsOverseas(false) //�Ƿ��Ǻ���ҵ��Ĭ��false
		.SetRootPathName(ROOT_PATH_NAME)
		.SetDid(device_id.c_str())
		.SetUid(device_id.c_str())
		.SetAppVersion("8.0.6")
		.SetChannel("pico_streaming")
		.SetReportInterval(1000)
		.SetMaxFileSize(1024*32); //�Ǳ���
	parfait_wrapper_->InitGlobalEnv(*global_env_builder);
	DestroyParfaitGlobalEnvBuilder(global_env_builder); //������ע�룬����ȫ�ֻ�������builder

	//����ʵ����������builder
	parfait::ParfaitEnvBuilderBase* env_builder = CreateParfaitEnvBuilder(PICOAPPID, INSTANCE_NAME);
	//����ʵ����������ֵ&��ʼ��ʵ����������
	env_builder->SetProcessName("steamvr");
	parfait_wrapper_->InitInstanceEnv(*env_builder);
	DestroyParfaitEnvBuilder(env_builder); //������ע�룬����ʵ����������builder
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
 