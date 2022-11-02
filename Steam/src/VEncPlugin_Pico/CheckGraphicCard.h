#ifndef CHECKGRAPHICCARD_H_
#define CHECKGRAPHICCARD_H_ "CheckGraphicCard.h"
#include <Windows.h>  
#include <iostream>  
#include <DXGI.h>  
#include <vector>  


using namespace std;

bool IfAMDCard()
{
	// ��������  
	IDXGIFactory * pFactory;
	IDXGIAdapter * pAdapter;
	std::vector <IDXGIAdapter*> vAdapters;            // �Կ�  


	// �Կ�������  
	int iAdapterNum = 0;


	// ����һ��DXGI����  
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));

	if (FAILED(hr))
		return false;

	// ö��������  
	while (pFactory->EnumAdapters(iAdapterNum, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		vAdapters.push_back(pAdapter);
		++iAdapterNum;
	}

	// ��Ϣ���   
	for (size_t i = 0; i < vAdapters.size(); i++)
	{
		// ��ȡ��Ϣ  
		DXGI_ADAPTER_DESC adapterDesc;
		vAdapters[i]->GetDesc(&adapterDesc);
		std::wstring aa(adapterDesc.Description);

		int p = aa.find(L"Radeon");
		if (aa.find(L"Radeon") != -1)
		{
			return true;
		}



		// ����Կ���Ϣ  
		//cout << "ϵͳ��Ƶ�ڴ�:" << adapterDesc.DedicatedSystemMemory / 1024 / 1024 << "M" << endl;
		//cout << "ר����Ƶ�ڴ�:" << adapterDesc.DedicatedVideoMemory / 1024 / 1024 << "M" << endl;
		//cout << "����ϵͳ�ڴ�:" << adapterDesc.SharedSystemMemory / 1024 / 1024 << "M" << endl;
		//cout << "�豸����:" << bb.c_str() << endl;
		//cout << "�豸ID:" << adapterDesc.DeviceId << endl;
		//cout << "PCI ID�����汾:" << adapterDesc.Revision << endl;
		//cout << "��ϵͳPIC ID:" << adapterDesc.SubSysId << endl;
		//cout << "���̱��:" << adapterDesc.VendorId << endl;

		//int p = 0;
	}
	return false;
}
bool IfNVIDIACard()
{
	// ��������  
	IDXGIFactory* pFactory;
	IDXGIAdapter* pAdapter;
	std::vector <IDXGIAdapter*> vAdapters;            // �Կ�  


	// �Կ�������  
	int iAdapterNum = 0;


	// ����һ��DXGI����  
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));

	if (FAILED(hr))
		return false;

	// ö��������  
	while (pFactory->EnumAdapters(iAdapterNum, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		vAdapters.push_back(pAdapter);
		++iAdapterNum;
	}

	// ��Ϣ���   
	for (size_t i = 0; i < vAdapters.size(); i++)
	{
		// ��ȡ��Ϣ  
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
