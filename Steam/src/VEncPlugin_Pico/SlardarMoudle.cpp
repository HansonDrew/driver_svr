#include "SlardarMoudle.h"

#include "../../../Slardar/header/ParfaitConstants.h"
#include "../../../Slardar/header/ParfaitEnvBase.h"
#include "../../../Slardar/header/ParfaitInstance.h"
#include "../../../Slardar/header/ParfaitRecorderBase.h"
#include "../../../Slardar/header/ParfaitWrapperBase.h"
#include "../../../Slardar/header/ParfaitLogRecorderBase.h"
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

	parfait::ParfaitWrapperBase* parfait_wrapper = CreateParfaitWrapper();

	//�Ƿ�debug��������parfait������ڲ�������־��Ĭ��false
	parfait::ParfaitGlobalEnvBuilderBase::SetIsDebug(true);
	//����ȫ�ֻ�������builder
	parfait::ParfaitGlobalEnvBuilderBase* global_env_builder = CreateParfaitGlobalEnvBuilder(1349);
	//����ȫ�ֻ�������ֵ&��ʼ��ȫ�ֻ�������
	global_env_builder
		->SetIsOverseas(true) //�Ƿ��Ǻ���ҵ��Ĭ��false
		.SetRootPathName(ROOT_PATH_NAME)
		.SetDid("12345")
		.SetUid("1234")
		.SetAppVersion("1.0.0")
		.SetChannel("app_store"); //�Ǳ���
	parfait_wrapper->InitGlobalEnv(*global_env_builder);
	DestroyParfaitGlobalEnvBuilder(global_env_builder); //������ע�룬����ȫ�ֻ�������builder

	//����ʵ����������builder
	parfait::ParfaitEnvBuilderBase* env_builder = CreateParfaitEnvBuilder(8619, INSTANCE_NAME);
	//����ʵ����������ֵ&��ʼ��ʵ����������
	env_builder->SetProcessName("main")
		.SetPid("1")
		.AddRecordContext("record_key", "record_value")//ע�������Ϣ��context
		.AddCrashContext("crash_key", "crash_value");
		parfait_wrapper->InitInstanceEnv(*env_builder);
	DestroyParfaitEnvBuilder(env_builder); //������ע�룬����ʵ����������builder
	return 1;
}