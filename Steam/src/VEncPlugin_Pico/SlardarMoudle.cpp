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

	//是否debug，开启后parfait会输出内部调试日志，默认false
	parfait::ParfaitGlobalEnvBuilderBase::SetIsDebug(true);
	//创建全局环境变量builder
	parfait::ParfaitGlobalEnvBuilderBase* global_env_builder = CreateParfaitGlobalEnvBuilder(1349);
	//设置全局环境参数值&初始化全局环境变量
	global_env_builder
		->SetIsOverseas(true) //是否是海外业务，默认false
		.SetRootPathName(ROOT_PATH_NAME)
		.SetDid("12345")
		.SetUid("1234")
		.SetAppVersion("1.0.0")
		.SetChannel("app_store"); //非必须
	parfait_wrapper->InitGlobalEnv(*global_env_builder);
	DestroyParfaitGlobalEnvBuilder(global_env_builder); //参数已注入，销毁全局环境变量builder

	//创建实例环境变量builder
	parfait::ParfaitEnvBuilderBase* env_builder = CreateParfaitEnvBuilder(8619, INSTANCE_NAME);
	//设置实例环境参数值&初始化实例环境变量
	env_builder->SetProcessName("main")
		.SetPid("1")
		.AddRecordContext("record_key", "record_value")//注入额外信息和context
		.AddCrashContext("crash_key", "crash_value");
		parfait_wrapper->InitInstanceEnv(*env_builder);
	DestroyParfaitEnvBuilder(env_builder); //参数已注入，销毁实例环境变量builder
	return 1;
}