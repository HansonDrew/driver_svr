#pragma once
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
				delete slardar_instance_;
				slardar_instance_ = nullptr;
			}
			 
		}
	};
	static SlardarMoudle* GetInstance();
	int Init();
};

