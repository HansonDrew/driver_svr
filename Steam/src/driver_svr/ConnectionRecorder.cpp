#include "ConnectionRecorder.h"

pxrConnectionRecorder* pxrConnectionRecorder::instance = nullptr;
pxrConnectionRecorder::AutoGC pxrConnectionRecorder::gc;


pxrConnectionRecorder* pxrConnectionRecorder::GetInstance()
{
	if (instance == nullptr)
    {
        instance = new pxrConnectionRecorder();
    }
    return instance;
}

void pxrConnectionRecorder::UpdateConnectionStatus(bool isConnected)
{
	if(!this->isConnected && isConnected)
	{
		isFirstConnected = true;
	}
    this->isConnected = isConnected;
}

bool pxrConnectionRecorder::GetIsConnected()
{
    return isConnected;
}

bool pxrConnectionRecorder::GetIsFirstConnected()
{
	bool val = isFirstConnected;
	if(isFirstConnected)
	{
		isFirstConnected = false;
	} //Make the flag to false, only if the first connected flag is get by user, otherwise stay true for later using.
	return val;
}

pxrConnectionRecorder::pxrConnectionRecorder()
{
}
