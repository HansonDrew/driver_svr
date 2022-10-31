//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#pragma comment( lib, "d3d11.lib" )

#pragma comment( lib, "D3DX11.lib" )
#include "D3DHelper.h"
#include <stdio.h>
#include<map>
#include<vector>
//#include "dxgi1_2.h"
#include "d3d11_4.h"
#include <d3d11.h> 
#include <D3Dx11tex.h>
 
namespace D3DHelper {

	HRESULT SaveTextureToFile(ID3D11DeviceContext*pContext, ID3D11Texture2D* TexToSave, LPCWSTR path)
	{
		
		HRESULT hr=NULL;

		hr = D3DX11SaveTextureToFile(pContext, TexToSave, D3DX11_IFF_JPG , path);
		 
		if (hr==E_FAIL)
		{
		}
		return hr;
	}
	HRESULT SaveTextureToFile(ID3D11Device*device, ID3D11Texture2D* TexToSave, LPCWSTR path)
	{
		 
		HRESULT hr = NULL;
		ID3D11DeviceContext* pContext;
		device->GetImmediateContext(&pContext);

		 D3DX11SaveTextureToFile(pContext, TexToSave, D3DX11_IFF_JPG, path);

		return hr;
	}
	bool FindDXGIOutput(IDXGIFactory *pFactory,  IDXGIAdapter **pOutAdapter, IDXGIOutput **pOutOutput, int32_t *pOutX, int32_t *pOutY)
	{
		IDXGIAdapter *pDXGIAdapter;
		for (UINT nAdapterIndex = 0; pFactory->EnumAdapters(nAdapterIndex, &pDXGIAdapter) != DXGI_ERROR_NOT_FOUND; nAdapterIndex++)
		{
			IDXGIOutput *pDXGIOutput;
			for (UINT nOutputIndex = 0; pDXGIAdapter->EnumOutputs(nOutputIndex, &pDXGIOutput) != DXGI_ERROR_NOT_FOUND; nOutputIndex++)
			{
				DXGI_OUTPUT_DESC desc;
				pDXGIOutput->GetDesc(&desc);

				//if ( desc.DesktopCoordinates.right - desc.DesktopCoordinates.left == nWidth &&
				//	desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top == nHeight )
				//{
				*pOutAdapter = pDXGIAdapter;
				*pOutOutput = pDXGIOutput;
				*pOutX = desc.DesktopCoordinates.left;
				*pOutY = desc.DesktopCoordinates.top;
				return true;
				//}
				pDXGIOutput->Release();
			}
			pDXGIAdapter->Release();
		}
		return false;
	}
#define FAILED(hr)      (((HRESULT)(hr)) < 0)
	//-----------------------------------------------------------------------------
	void CreateDevice(ID3D11Device*& device)
		//-----------------------------------------------------------------------------
	{
		
		IDXGIFactory1 *pDXGIFactory1;
		IDXGIFactory *m_pDXGIFactory = nullptr;
		if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)&pDXGIFactory1)))
		{
			 
			return;
		}
		else if (FAILED(pDXGIFactory1->QueryInterface(__uuidof(IDXGIFactory), (void **)&m_pDXGIFactory)))
		{
			pDXGIFactory1->Release();
			 
			return;
		}
		pDXGIFactory1->Release();
		
		IDXGIOutput *m_pDXGIOutput;
		IDXGIAdapter *pDXGIAdapter;
		/*int m_nDisplayX, m_nDisplayY;
		if (!FindDXGIOutput(m_pDXGIFactory,   &pDXGIAdapter, &m_pDXGIOutput, &m_nDisplayX, &m_nDisplayY))
			return ;*/
			//IDXGIAdapter *pDXGIAdapter;
	   m_pDXGIFactory->EnumAdapters(0, &pDXGIAdapter);
	 
        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
        ID3D11DeviceContext* pD3D11Context;

		// Create the device and device context objects
		D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&device,
			&featureLevel,
            &pD3D11Context);
		 
        ID3D11Multithread *D3D11Multithread = NULL;
        HRESULT hr = pD3D11Context->QueryInterface(__uuidof(ID3D11Multithread), (void **)&D3D11Multithread);
        if (SUCCEEDED(hr)) {
            D3D11Multithread->SetMultithreadProtected(TRUE);
            D3D11Multithread->Release();
        }
	}

	//-----------------------------------------------------------------------------
	void CreateDevice0(ID3D11Device*& device, ID3D11DeviceContext*& pD3D11Context)
		//-----------------------------------------------------------------------------
	{

		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		 

		// Create the device and device context objects
		HRESULT HR=D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&device,
			&featureLevel,
			&pD3D11Context);
		 
		ID3D11Multithread *D3D11Multithread = NULL;
		HRESULT hr = pD3D11Context->QueryInterface(__uuidof(ID3D11Multithread), (void **)&D3D11Multithread);
		if (SUCCEEDED(hr)) {
			D3D11Multithread->SetMultithreadProtected(TRUE);
			D3D11Multithread->Release();
		}
		
	}

    //-----------------------------------------------------------------------------
    IDXGIKeyedMutex* AcquireMutex(ID3D11Device* d3d11Device, HANDLE syncTexture)
    //-----------------------------------------------------------------------------
    {
        IDXGIKeyedMutex *pKeyedMutex = nullptr;
        ID3D11Texture2D* pSyncTexture = D3DHelper::AsTexture(d3d11Device, (HANDLE)(syncTexture));
        if (pSyncTexture != nullptr)
        {
            if (SUCCEEDED(pSyncTexture->QueryInterface(__uuidof(IDXGIKeyedMutex), (void **)&pKeyedMutex)))
            {
                if (pKeyedMutex->AcquireSync(0, 10) != S_OK)
                {
                    pKeyedMutex->Release();
                    pKeyedMutex = nullptr;
                }
            }
        }

        return pKeyedMutex;
    }

    //-----------------------------------------------------------------------------
    void ReleaseMutex(IDXGIKeyedMutex* pKeyedMutex)
    //-----------------------------------------------------------------------------
    {
        if (pKeyedMutex)
        {
            pKeyedMutex->ReleaseSync(0);
            pKeyedMutex->Release();
        }
    }

    //-----------------------------------------------------------------------------
    void Clear(ID3D11DeviceContext* deviceContext, ID3D11RenderTargetView* rt, float* color)
    //-----------------------------------------------------------------------------
    {
        deviceContext->ClearRenderTargetView(rt, color);
    }

	//-----------------------------------------------------------------------------
	HANDLE CreateSharedResource(ID3D11Texture2D* texture)
	//-----------------------------------------------------------------------------
	{
		HANDLE handle;
		IDXGIResource1* pResource;
		texture->QueryInterface(__uuidof(IDXGIResource1), (void**)&pResource);
        pResource->CreateSharedHandle(NULL,
            DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE,
            NULL, &handle);                            
		pResource->Release();
		return handle;
	}

    //-----------------------------------------------------------------------------
    HANDLE CreateSharedResource0(ID3D11Texture2D* texture)
        //-----------------------------------------------------------------------------
    {
        HANDLE handle;
        IDXGIResource1* pResource;
        texture->QueryInterface(__uuidof(IDXGIResource1), (void**)&pResource);
        pResource->GetSharedHandle(&handle);
        pResource->Release();
        return handle;
    }

	//-----------------------------------------------------------------------------
	void GetTextureInfo(ID3D11Texture2D* texture, DXGI_FORMAT* format, float* width, float* height)
	//-----------------------------------------------------------------------------
	{
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);
		if (width != nullptr)
			*width = (float)desc.Width;
		if (height != nullptr)
			*height = (float)desc.Height;
		if (format != nullptr)
			*format = desc.Format;
	}

    //-----------------------------------------------------------------------------
    ID3D11Texture2D* CreateSharedTexture2D(ID3D11Device* device, DXGI_FORMAT format, int width, int height, int nSampleCount)
        //-----------------------------------------------------------------------------
    {
        ID3D11Texture2D* texture = nullptr;

        DXGI_SAMPLE_DESC sampledesc;
        sampledesc.Count = nSampleCount;
        sampledesc.Quality = 0;

        D3D11_TEXTURE2D_DESC desc;
        desc.ArraySize = 1;
        desc.CPUAccessFlags = 0;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        desc.Format = format;
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
        desc.SampleDesc = sampledesc;
        desc.Usage = D3D11_USAGE_DEFAULT;
        device->CreateTexture2D(&desc, 0, &texture);

        return texture;
    }
	uint32_t ColorRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		return (r | (g << 8) | (b << 16) | (a << 24));
	}


	ID3D11Texture2D * CreateSaveTexture2D(ID3D11Device * device, DXGI_FORMAT format, int width, int height, int nSampleCount)
	{
		ID3D11Texture2D* texture = nullptr;

		DXGI_SAMPLE_DESC sampledesc;
		sampledesc.Count = nSampleCount;
		sampledesc.Quality = 0;

		D3D11_TEXTURE2D_DESC desc;
		desc.ArraySize = 1;
		desc.CPUAccessFlags = 0;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.Format = format;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
		desc.SampleDesc = sampledesc;
		desc.Usage = D3D11_USAGE_DEFAULT;
		uint32_t black = ColorRGBA(0, 0, 0, 255), orange = ColorRGBA(255, 108, 0, 255);

		// 纹理内存映射，用黑色初始化width*height+width*height/2
		std::vector<uint32_t> textureMap(width * height, orange);
		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		uint32_t * pData = textureMap.data();
		sd.pSysMem = pData;
		sd.SysMemPitch = width * sizeof(uint32_t);
		sd.SysMemSlicePitch = width *  height* sizeof(uint32_t) ;
		
		
		HRESULT hr=device->CreateTexture2D(&desc, &sd, &texture);
		if (FAILED(hr)) exit(0);
		return texture;
	}

	//-----------------------------------------------------------------------------
	ID3D11Texture2D* CreateTexture2D(ID3D11Device* device, DXGI_FORMAT format, int width, int height, int nSampleCount)
	//-----------------------------------------------------------------------------
	{
		ID3D11Texture2D* texture = nullptr;

		DXGI_SAMPLE_DESC sampledesc;
		sampledesc.Count = nSampleCount;
		sampledesc.Quality = 0;

		D3D11_TEXTURE2D_DESC desc;
		desc.ArraySize = 1;
		desc.CPUAccessFlags =0;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;//
		desc.Format = format;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
        desc.MiscFlags = 0;
		desc.SampleDesc = sampledesc;
		desc.Usage = D3D11_USAGE_DEFAULT;
		device->CreateTexture2D(&desc, 0, &texture);

		return texture;
	}

	void DeleteTexture2D(ID3D11Texture2D* texture)
	{
        if (texture != nullptr)
        {
            texture->Release();
        }
	}

	void DeleteSharedResource(HANDLE handle)
	{

	}

	static std::map<HANDLE, ID3D11Texture2D*> shMap;
	//-----------------------------------------------------------------------------
	ID3D11Texture2D* AsTexture(ID3D11Device* device, HANDLE hSharedTexture)
	//-----------------------------------------------------------------------------
	{
		if (!hSharedTexture)
			return NULL;

		ID3D11Texture2D* pTexture;
		if (shMap.find(hSharedTexture) != shMap.end())
		{
			pTexture = shMap[(HANDLE)hSharedTexture];
		}
		else {
			HRESULT hr = device->OpenSharedResource((HANDLE)hSharedTexture, __uuidof(ID3D11Texture2D), (void **)&pTexture);
            if (SUCCEEDED(hr))
            {
                shMap[hSharedTexture] = pTexture;
            }
            else {
               // DriverLog("\nFailed to Mapp Shared Resouce!!!!!!!!!!!!!!!!!!!!!\n");
            }
		}

		return pTexture;
	}
}
//HRESULT WINAPI DXTraceW(_In_z_ const WCHAR* strFile, _In_ DWORD dwLine, _In_ HRESULT hr,
//	_In_opt_ const WCHAR* strMsg, _In_ bool bPopMsgBox)
//{
//	WCHAR strBufferFile[MAX_PATH];
//	WCHAR strBufferLine[128];
//	WCHAR strBufferError[300];
//	WCHAR strBufferMsg[1024];
//	WCHAR strBufferHR[40];
//	WCHAR strBuffer[3000];
//
//	swprintf_s(strBufferLine, 128, L"%lu", dwLine);
//	if (strFile)
//	{
//		swprintf_s(strBuffer, 3000, L"%ls(%ls): ", strFile, strBufferLine);
//		//OutputDebugStringW(strBuffer);
//	}
//
//	size_t nMsgLen = (strMsg) ? wcsnlen_s(strMsg, 1024) : 0;
//	if (nMsgLen > 0)
//	{
//		//OutputDebugStringW(strMsg);
//		//OutputDebugStringW(L" ");
//	}
//	// Windows SDK 8.0起DirectX的错误信息已经集成进错误码中，可以通过FormatMessageW获取错误信息字符串
//	// 不需要分配字符串内存
//	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
//		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//		strBufferError, 256, nullptr);
//
//	WCHAR* errorStr = wcsrchr(strBufferError, L'\r');
//	if (errorStr)
//	{
//		errorStr[0] = L'\0';	// 擦除FormatMessageW带来的换行符(把\r\n的\r置换为\0即可)
//	}
//
//	swprintf_s(strBufferHR, 40, L" (0x%0.8x)", hr);
//	wcscat_s(strBufferError, strBufferHR);
//	swprintf_s(strBuffer, 3000, L"错误码含义：%ls", strBufferError);
//	//OutputDebugStringW(strBuffer);
//
//	//OutputDebugStringW(L"\n");
//
//	if (bPopMsgBox)
//	{
//		wcscpy_s(strBufferFile, MAX_PATH, L"");
//		if (strFile)
//			wcscpy_s(strBufferFile, MAX_PATH, strFile);
//
//		wcscpy_s(strBufferMsg, 1024, L"");
//		if (nMsgLen > 0)
//			swprintf_s(strBufferMsg, 1024, L"当前调用：%ls\n", strMsg);
//
//		swprintf_s(strBuffer, 3000, L"文件名：%ls\n行号：%ls\n错误码含义：%ls\n%ls您需要调试当前应用程序吗？",
//			strBufferFile, strBufferLine, strBufferError, strBufferMsg);
//
//	/*	int nResult = MessageBoxW(GetForegroundWindow(), strBuffer, L"错误", MB_YESNO | MB_ICONERROR);
//		if (nResult == IDYES)
//			DebugBreak();*/
//	}
//
//	return hr;
//}