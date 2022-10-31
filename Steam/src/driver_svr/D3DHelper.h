//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#pragma once

#include <d3d11.h>
#include <stdio.h>

#define convertinencoder
namespace D3DHelper {
	
	HRESULT SaveTextureToFile(ID3D11Device*device, ID3D11Texture2D* TexToSave, LPCWSTR path);
	HRESULT SaveTextureToFile(ID3D11DeviceContext*pContext, ID3D11Texture2D* TexToSave, LPCWSTR path);
	void CreateDevice(ID3D11Device*&,  IDXGIAdapter* pAdapter = NULL);
	void CreateDevice0(ID3D11Device*& test, ID3D11DeviceContext*& pD3D11Context);
	HANDLE CreateSharedResource(ID3D11Texture2D* texture);
    HANDLE CreateSharedResource0(ID3D11Texture2D* texture);
	void GetTextureInfo(ID3D11Texture2D* texture, DXGI_FORMAT* format, float* width, float* height);
	ID3D11Texture2D* CreateTexture2D(ID3D11Device* device, DXGI_FORMAT format, int width, int height, int nSampleCount);
    ID3D11Texture2D* CreateSharedTexture2D(ID3D11Device* device, DXGI_FORMAT format, int width, int height, int nSampleCount,  bool keyedmutex = false);
	ID3D11Texture2D* CreateSaveTexture2D(ID3D11Device* device, DXGI_FORMAT format, int width, int height, int nSampleCount);
	ID3D11Texture2D* CreateSharedMipMapTexture2D(ID3D11Device* device, DXGI_FORMAT format, int width, int height, int nSampleCount, bool keyedmutex = false);
	void DeleteTexture2D(ID3D11Texture2D*);
	void DeleteSharedResource(HANDLE);
	ID3D11Texture2D* AsTexture(ID3D11Device* device, HANDLE hSharedTexture);
    void Clear(ID3D11DeviceContext* deviceContext, ID3D11RenderTargetView* rt, float* color);
    IDXGIKeyedMutex* AcquireMutex(ID3D11Device* d3d11Device, HANDLE syncTexture);
	IDXGIKeyedMutex* AcquireMutexByTexture( ID3D11Texture2D* texture);
    void ReleaseMutex(IDXGIKeyedMutex* pKeyedMutex);
	IDXGIAdapter* FindIndependent();
}

HRESULT WINAPI DXTraceW(_In_z_ const WCHAR* strFile, _In_ DWORD dwLine, _In_ HRESULT hr, _In_opt_ const WCHAR* strMsg, _In_ bool bPopMsgBox);

// ------------------------------
// HRºê
// ------------------------------
// 
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)												\
	{															\
		HRESULT hr = (x);										\
		if(FAILED(hr))											\
		{														\
			DXTraceW(__FILEW__, (DWORD)__LINE__, hr, L#x, true);\
		}														\
	}
#endif
#else
#ifndef HR
#define HR(x) (x)
#endif 
#endif

