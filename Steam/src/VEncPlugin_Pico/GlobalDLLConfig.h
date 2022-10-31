#pragma once
#include <atomic>

class GlobalDLLConfig
{
	
public:
	GlobalDLLConfig() = default;

	GlobalDLLConfig(const GlobalDLLConfig& other);


	GlobalDLLConfig(GlobalDLLConfig&& other) noexcept;


	GlobalDLLConfig& operator=(const GlobalDLLConfig& other);


	GlobalDLLConfig& operator=(GlobalDLLConfig&& other) noexcept;


	int GetGOPSize() const
	{
		return gopSize;
	}

	void SetGOPSize(int gop_size)
	{
		gopSize = gop_size;
	}

	int GetRateControllMode() const
	{
		return rateControllMode;
	}

	void SetRateControllMode(int rate_controll_mode)
	{
		rateControllMode = rate_controll_mode;
	}

	int GetFixedFoveatedEncode() const
	{
		return fixedFoveatedEncode.load();
	}

	void SetFixedFoveatedEncode(int fixed_foveated_encode)
	{
		fixedFoveatedEncode.store(fixed_foveated_encode);
	}

private:
	int gopSize = 0;
	int rateControllMode = 0;
	std::atomic_int fixedFoveatedEncode = 0;
};

