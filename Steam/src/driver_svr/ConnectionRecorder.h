#pragma once

class pxrConnectionRecorder
{

public:
	static pxrConnectionRecorder* GetInstance();

	void UpdateConnectionStatus(bool isConnected);
	bool GetIsConnected();
	bool GetIsFirstConnected();

private:

	bool isConnected = false;
	bool isFirstConnected = false;
	
	pxrConnectionRecorder();
	static pxrConnectionRecorder* instance;

	class AutoGC
	{
	public:
		AutoGC() = default;
		~AutoGC() { if (instance != nullptr) { delete instance; instance = nullptr; } };
	};
	static AutoGC gc;

};


