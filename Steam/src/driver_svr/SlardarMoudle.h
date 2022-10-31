#pragma once
#ifndef NO_SLARDAR
#include <string>
#include <vector>
#include <mutex>
#include "../../../Slardar/header/ParfaitConstants.h"
#include "../../../Slardar/header/ParfaitEnvBase.h"
#include "../../../Slardar/header/ParfaitInstance.h"
#include "../../../Slardar/header/ParfaitRecorderBase.h"
#include "../../../Slardar/header/ParfaitWrapperBase.h"
#include "../../../Slardar/header/ParfaitLogRecorderBase.h"
#define PICOAPPID 8619
#define SLARDAR_BUF_SIZE 20
class SlardarMoudle
{
private:
	SlardarMoudle() {};
	~SlardarMoudle() {};
	static SlardarMoudle *slardar_instance_ ;
public:
	class GC  
	{
	public:
		GC()
		{
		}
		~GC()
		{
			 
			if (slardar_instance_ != nullptr)
			{
				slardar_instance_->ClearUp();
				delete slardar_instance_;
				slardar_instance_ = nullptr;
			}
			 
		}
	};
	static SlardarMoudle* GetInstance();
	static GC gc;
	int Init();
	void ClearUp();
	void SetTaskId(std::string taskid) { task_id_ = "\""+taskid+ "\""; };
	std::string GetTaskId() { return task_id_; };
	int SendToSlardar();
	void SaveMessage(std::string message);
	int GetMessageBufSize() { return message_buf_.size(); };
private:
	parfait::ParfaitWrapperBase* parfait_wrapper_=nullptr;
	parfait::ParfaitUserDefinedRecorderBase* recorder_ = nullptr;
	std::vector<std::string>message_buf_;
	std::mutex buf_mutex_;
	std::string task_id_="streaming_1234";
};

#endif