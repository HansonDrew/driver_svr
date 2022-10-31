#ifndef LogTool_h__
#define LogTool_h__
/*!
 * \file pxrLogTool.h
 * \date 2019/02/15 15:33
 *
 * \author nico.liu
 * Contact: nico.liu@picovr.com
 *
 * \brief PicoVR Log Tool
 *		
 * TODO: 
 *
 * \note Take care of the log path if use this.
*/

#include <string>
#include "../RVRPlugin/IVEncPlugin.h"

class pxrLogTool
{

	enum pxrLogLevel {
		PXR_LOG_BUFFER,
		PXR_LOG_VERBOSE,
		PXR_LOG_INFO,
		PXR_LOG_KPI,
		PXR_LOG_WARN,
		PXR_LOG_ERROR,
		PXR_LOG_ALWAYS,
		PXR_LOG_NONE
	};
	
public:
	explicit pxrLogTool();
	~pxrLogTool();


	void Log(const std::string& Msg, pxrLogLevel Level);


	void LogTrace(const std::string& Msg);

	
	void LogInfo(const std::string& Msg);

	
	void LogWarning(const std::string& Msg);

	
	void LogError(const std::string& Msg);

	
	void LogAlways(const std::string& Msg);

	void LogSlardar (const std::string& Msg);

};



#endif // LogTool_h__
