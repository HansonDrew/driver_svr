#ifndef CHECKGRAPHICCARD_H_
#define CHECKGRAPHICCARD_H_ "CheckGraphicCard.h"
#include <Windows.h>  
#include <iostream>  
#include <DXGI.h>  
#include <vector>  


using namespace std;

bool IfAMDCard()
{
	// 参数定义  
	IDXGIFactory * pFactory;
	IDXGIAdapter * pAdapter;
	std::vector <IDXGIAdapter*> vAdapters;            // 显卡  


	// 显卡的数量  
	int iAdapterNum = 0;


	// 创建一个DXGI工厂  
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));

	if (FAILED(hr))
		return false;

	// 枚举适配器  
	while (pFactory->EnumAdapters(iAdapterNum, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		vAdapters.push_back(pAdapter);
		++iAdapterNum;
	}

	// 信息输出   
	for (size_t i = 0; i < vAdapters.size(); i++)
	{
		// 获取信息  
		DXGI_ADAPTER_DESC adapterDesc;
		vAdapters[i]->GetDesc(&adapterDesc);
		std::wstring aa(adapterDesc.Description);

		int p = aa.find(L"Radeon");
		if (aa.find(L"Radeon") != -1)
		{
			return true;
		}



		// 输出显卡信息  
		//cout << "系统视频内存:" << adapterDesc.DedicatedSystemMemory / 1024 / 1024 << "M" << endl;
		//cout << "专用视频内存:" << adapterDesc.DedicatedVideoMemory / 1024 / 1024 << "M" << endl;
		//cout << "共享系统内存:" << adapterDesc.SharedSystemMemory / 1024 / 1024 << "M" << endl;
		//cout << "设备描述:" << bb.c_str() << endl;
		//cout << "设备ID:" << adapterDesc.DeviceId << endl;
		//cout << "PCI ID修正版本:" << adapterDesc.Revision << endl;
		//cout << "子系统PIC ID:" << adapterDesc.SubSysId << endl;
		//cout << "厂商编号:" << adapterDesc.VendorId << endl;

		//int p = 0;
	}
	return false;
}
bool IfNVIDIACard()
{
	// 参数定义  
	IDXGIFactory* pFactory;
	IDXGIAdapter* pAdapter;
	std::vector <IDXGIAdapter*> vAdapters;            // 显卡  


	// 显卡的数量  
	int iAdapterNum = 0;


	// 创建一个DXGI工厂  
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));

	if (FAILED(hr))
		return false;

	// 枚举适配器  
	while (pFactory->EnumAdapters(iAdapterNum, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		vAdapters.push_back(pAdapter);
		++iAdapterNum;
	}

	// 信息输出   
	for (size_t i = 0; i < vAdapters.size(); i++)
	{
		// 获取信息  
		DXGI_ADAPTER_DESC adapterDesc;
		vAdapters[i]->GetDesc(&adapterDesc);
		std::wstring aa(adapterDesc.Description);
	 
		if (aa.find(L"NVIDIA") != -1)
		{
			return true;
		}

		 
	}
	return false;
}
#endif
