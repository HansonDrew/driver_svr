#include "GlobalDLLConfig.h"

GlobalDLLConfig::GlobalDLLConfig(const GlobalDLLConfig& other)
	: gopSize(other.gopSize),
	rateControllMode(other.rateControllMode),
	fixedFoveatedEncode(other.fixedFoveatedEncode.load())
{
}

GlobalDLLConfig::GlobalDLLConfig(GlobalDLLConfig&& other) noexcept
	: gopSize(other.gopSize),
	rateControllMode(other.rateControllMode),
	fixedFoveatedEncode(other.fixedFoveatedEncode.load())
{
}

GlobalDLLConfig& GlobalDLLConfig::operator=(const GlobalDLLConfig& other)
{
	if (this == &other)
		return *this;
	gopSize = other.gopSize;
	rateControllMode = other.rateControllMode;
	fixedFoveatedEncode = other.fixedFoveatedEncode.load();
	return *this;
}

GlobalDLLConfig& GlobalDLLConfig::operator=(GlobalDLLConfig&& other) noexcept
{
	if (this == &other)
		return *this;
	gopSize = other.gopSize;
	rateControllMode = other.rateControllMode;
	fixedFoveatedEncode = other.fixedFoveatedEncode.load();
	return *this;
}
