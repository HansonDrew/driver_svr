// "DebugHelper"
// Copyright (c) 2019.7 PICO.  All rights reserved.
// Functions for Help debug
// C++ header
// Create by Gavin.Yu
#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <D3Dx11tex.h>  
#include<stdio.h>
#pragma comment( lib, "D3DX11.lib" )
HRESULT WINAPI DXTraceW(_In_z_ const WCHAR* strFile, _In_ DWORD dwLine, _In_ HRESULT hr, _In_opt_ const WCHAR* strMsg, _In_ bool bPopMsgBox);

// ------------------------------
// HR宏
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

HRESULT WINAPI DXTraceW(_In_z_ const WCHAR* strFile, _In_ DWORD dwLine, _In_ HRESULT hr,
	_In_opt_ const WCHAR* strMsg, _In_ bool bPopMsgBox)
{
	WCHAR strBufferFile[MAX_PATH];
	WCHAR strBufferLine[128];
	WCHAR strBufferError[300];
	WCHAR strBufferMsg[1024];
	WCHAR strBufferHR[40];
	WCHAR strBuffer[3000];

	swprintf_s(strBufferLine, 128, L"%lu", dwLine);
	if (strFile)
	{
		swprintf_s(strBuffer, 3000, L"%ls(%ls): ", strFile, strBufferLine);
		//OutputDebugStringW(strBuffer);
	}

	size_t nMsgLen = (strMsg) ? wcsnlen_s(strMsg, 1024) : 0;
	if (nMsgLen > 0)
	{
		//OutputDebugStringW(strMsg);
		//OutputDebugStringW(L" ");
	}
	// Windows SDK 8.0起DirectX的错误信息已经集成进错误码中，可以通过FormatMessageW获取错误信息字符串
	// 不需要分配字符串内存
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		strBufferError, 256, nullptr);

	WCHAR* errorStr = wcsrchr(strBufferError, L'\r');
	if (errorStr)
	{
		errorStr[0] = L'\0';	// 擦除FormatMessageW带来的换行符(把\r\n的\r置换为\0即可)
	}

	swprintf_s(strBufferHR, 40, L" (0x%0.8x)", hr);
	wcscat_s(strBufferError, strBufferHR);
	swprintf_s(strBuffer, 3000, L"错误码含义：%ls", strBufferError);
	//OutputDebugStringW(strBuffer);

	//OutputDebugStringW(L"\n");

	if (bPopMsgBox)
	{
		wcscpy_s(strBufferFile, MAX_PATH, L"");
		if (strFile)
			wcscpy_s(strBufferFile, MAX_PATH, strFile);

		wcscpy_s(strBufferMsg, 1024, L"");
		if (nMsgLen > 0)
			swprintf_s(strBufferMsg, 1024, L"当前调用：%ls\n", strMsg);

		swprintf_s(strBuffer, 3000, L"文件名：%ls\n行号：%ls\n错误码含义：%ls\n%ls您需要调试当前应用程序吗？",
			strBufferFile, strBufferLine, strBufferError, strBufferMsg);

		int nResult = MessageBoxW(GetForegroundWindow(), strBuffer, L"错误", MB_YESNO | MB_ICONERROR);
		if (nResult == IDYES)
			DebugBreak();
	}

	return hr;
}
#if defined(UNICODE) | defined(_UNICODE)
HRESULT SaveTextureToFile(ID3D11DeviceContext*pContext, ID3D11Texture2D* TexToSave, LPWSTR path)
{
	HRESULT hr;

	hr = D3DX11SaveTextureToFile(pContext, TexToSave, D3DX11_IFF_JPG, path);
	if (FAILED(hr))
	{
		//OutputDebugStringA("DXResourceManager: SaveTextureToFile Failed!\n");
	}

	return hr;
}
#else
HRESULT SaveTextureToFile(ID3D11DeviceContext*pContext, ID3D11Texture2D* TexToSave, LPCSTR path)
{
	HRESULT hr;

	hr = D3DX11SaveTextureToFile(pContext, TexToSave, D3DX11_IFF_JPG, path);
	if (FAILED(hr))
	{
		//OutputDebugStringA("DXResourceManager: SaveTextureToFile Failed!\n");
	}

	return hr;
}

#endif
