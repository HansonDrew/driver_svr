
#include "LogTool.h"
#include "../RVRUtils/RVRLogger.h"
extern LogFun gLogFun;
pxrLogTool::pxrLogTool()
{

}


pxrLogTool::~pxrLogTool()
{
}



void pxrLogTool::Log(const std::string& Msg, pxrLogLevel Level)
{
	int level = Level;
	RVR::Log(RVR::RVRLogLevel(level), "PicoVEncPlugin","%s", Msg.c_str());
}

void pxrLogTool::LogTrace(const std::string& Msg)
{
	Log(Msg, pxrLogLevel::PXR_LOG_VERBOSE);
}



void pxrLogTool::LogInfo(const std::string& Msg)
{
	Log(Msg, pxrLogLevel::PXR_LOG_INFO);
}


void pxrLogTool::LogWarning(const std::string& Msg)
{
	Log(Msg, pxrLogLevel::PXR_LOG_WARN);
}


void pxrLogTool::LogError(const std::string& Msg)
{
	Log(Msg, pxrLogLevel::PXR_LOG_ERROR);
}


void pxrLogTool::LogAlways(const std::string& Msg)
{
	Log(Msg, pxrLogLevel::PXR_LOG_ALWAYS);
}

void pxrLogTool::LogSlardar(const std::string& Msg) 
{
	std::string msg = "slardar";
	msg = msg + Msg;
	gLogFun(msg.c_str(), msg.length());
}

