//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#include "Compositor.h"
#include "D3DHelper.h"

#include "Blit2.h"
#include <map>
#include "driverlog.h"
#include "config_reader.h"
#include "RVRLogger.h"
extern ConfigReader gConfigReader;
 
using namespace RVR;
extern bool gPrintMsg;
extern bool gLog;
//-----------------------------------------------------------------------------
bool Compositor::Initialize(ID3D11Device* d3d11Device, int width, int height)
//-----------------------------------------------------------------------------
{
	init_ = true;
    DriverLog("______COMPOSITOR_INITIALIZE______");
	
    this->Width = width;
    this->Height = height;
	this->device = d3d11Device;
   // device->AddRef();
	device->GetImmediateContext(&context);
	layers = new std::vector<LayerDesc>();
	
	//Create 
	for (int i = 0; i < 2; i++)
	{
		//DXGI_FORMAT_B8G8R8A8_UNORM
		if (gConfigReader.BigPicture()==0)
		{
			resultTexture[i] = D3DHelper::CreateSharedTexture2D(device, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, width, height, 1);
			device->CreateRenderTargetView(resultTexture[i], NULL, &resultRenderTarget[i]);
		}
		else
		{
			 
			resultTexture2to1[i] = D3DHelper::CreateSharedTexture2D(device, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Width, height, 1);
			device->CreateRenderTargetView(resultTexture2to1[i], NULL, &resultRenderTarget2to1[i]);
			
		}
		//以下是深度检测
		shared_texture_[i] = D3DHelper::CreateSharedTexture2D(device, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, width, height, 1, true);
		shared_texture_handle_[i] = D3DHelper::CreateSharedResource0(shared_texture_[i]);
		
		//resultNVTexture[i] = D3DHelper::CreateTexture2D(device, DXGI_FORMAT::DXGI_FORMAT_NV12, width, height, 1);

#ifdef PICO_PRESENT_MODE
		encodeTexture[i] = D3DHelper::CreateTexture2D(device, DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM, width, height, 1);
#endif
		
		::ZeroMemory(&resultViewport, sizeof(resultViewport));
		if (gConfigReader.BigPicture()==0)
		{
			D3DHelper::GetTextureInfo(resultTexture[0], (DXGI_FORMAT*)nullptr, &resultViewport.Width, &resultViewport.Height);
		} 
		else
		{
			D3DHelper::GetTextureInfo(resultTexture2to1[0], (DXGI_FORMAT*)nullptr, &resultViewport.Width, &resultViewport.Height);
		}
		
		resultViewport.MinDepth = 0.0f;
		resultViewport.MaxDepth = 1.0f;
		resultViewport.TopLeftX = 0;
		resultViewport.TopLeftY = 0;
		
		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory(&descDepth, sizeof(descDepth));
		descDepth.Width = width;
		descDepth.Height = height;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;

		device->CreateTexture2D(&descDepth, nullptr, &mDepthStencil[i]);
		

		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		device->CreateDepthStencilView(mDepthStencil[i], &descDSV, &mDepthStencilView[i]);

	}
 
	if (gConfigReader.GetSplit_() == 1)
	{
		for (int i=0;i<SplitTextureSize;i++)
		{
			split_texture_[i] = D3DHelper::CreateSharedTexture2D(device, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Width, height, 1, true);
			split_texture_handle_[i] = D3DHelper::CreateSharedResource0(split_texture_[i]);
		}
	}

#ifdef picomode 
	if (gConfigReader.GetAADTFlag() == 1)
	{
		mAADT.CreateAADTResources(device, 50, 50, gConfigReader.GetAADTFlag(), height, gConfigReader.GetEveHeight());
	}
	else
	{
		Blit2::CreateBlit2Resources(device);

		if (gConfigReader.BigPicture() == 0)
		{
			Blit2::CreateTextResources(resultTexture[0]);
		}
		else
		{
			Blit2::CreateTextResources(resultTexture2to1[0]);
		}
	}

	//Blit::CreateBlitResources(device);
#else
	Blit::CreateBlitResources(device);
#endif
	

	return true;
}
#ifdef PICO_PRESENT_MODE
void Compositor::Save()
{
	for (int i = 0; i < 2; i++)
	{
		context->CopyResource(encodeTexture[i], resultTexture[i]);
	}
}

ID3D11Texture2D* Compositor::GetEncode(int index)
{
	ID3D11Texture2D* texture = nullptr;

	if ((index >= 0) && (index < 2))
	{
		texture = encodeTexture[index];
	}
	return texture;
}
#endif
//-----------------------------------------------------------------------------
void Compositor::Add(LayerDesc* layerInfo)
//-----------------------------------------------------------------------------
{
	layers->push_back(*layerInfo);
}

//-----------------------------------------------------------------------------
void Compositor::Merge()
//-----------------------------------------------------------------------------
{
    float backgroundColor[4] = {0.01f, 0.01f, 0.01f, 1.0f};

    D3DHelper::Clear(context, resultRenderTarget[0], backgroundColor);
    D3DHelper::Clear(context, resultRenderTarget[1], backgroundColor);

	size_t count = layers->size();

	for (int i = 0; i < count; i++)
	{		 
		if (layers->at(i).left.texture != nullptr)
		{
#ifdef picomode
			
			Blit2::Blit2(resultRenderTarget[0], &resultViewport, AsShaderResource(device, layers->at(i).left.texture), mDepthStencilView[0], layers->at(i).left.bounds, !(i == 0));
			
#else
			Blit::Blit(resultRenderTarget[0], &resultViewport, AsShaderResource(device, layers->at(i).left.texture), layers->at(i).left.bounds, !(i == 0));
#endif
          
		}
		if (layers->at(i).right.texture != nullptr)
		{
#ifdef picomode
		
			Blit2::Blit2(resultRenderTarget[1], &resultViewport, AsShaderResource(device, layers->at(i).right.texture), mDepthStencilView[1], layers->at(i).right.bounds, !(i == 0));
					
#else
			Blit::Blit(resultRenderTarget[1], &resultViewport, AsShaderResource(device, layers->at(i).right.texture), layers->at(i).right.bounds, !(i==0));
#endif
		}
	}
	if (gPrintMsg)
	{
		Blit2::WirteText(L"pico test");
	}
	
    Clear();
}

void Compositor::SetQ1Q2(float q1[4], float q2[4]) 
{
	Blit2::SetQ1Q2(q1,q2);
}
//-----------------------------------------------------------------------------
void Compositor::Merge2to1(int index)
//-----------------------------------------------------------------------------
{
	float backgroundColor[4] = { 0.01f, 0.01f, 0.01f, 1.0f };
	if (index==0)
	{
		D3DHelper::Clear(context, resultRenderTarget2to1[0], backgroundColor);
	}
	else if (index == 1) 
	{
		D3DHelper::Clear(context, resultRenderTarget2to1[1], backgroundColor);
	}
	
	

	size_t count = layers->size();

	for (int i = 0; i < count; i++)
	{
		if (layers->at(i).left.texture != nullptr&& layers->at(i).right.texture !=nullptr)
		{

			if (gConfigReader.GetAADTFlag() == 0)
			{
				ID3D11ShaderResourceView* src[2];
				src[0] = AsShaderResource(device, layers->at(i).left.texture, -1);
				src[1] = AsShaderResource(device, layers->at(i).right.texture, -1);
				Blit2::Blit2to1(resultRenderTarget2to1[index], &resultViewport, src, layers->at(i).left.bounds, layers->at(i).right.bounds, !(i == 0));
				//DriverLog("blit layer %d  leftbounds %f %f %f %f \n",i, layers->at(i).left.bounds[0], layers->at(i).left.bounds[1], layers->at(i).left.bounds[2], layers->at(i).left.bounds[3]);
			}
			else
			{
				ID3D11ShaderResourceView* src[2];
				src[0] = AsShaderResource(device, layers->at(i).left.texture, -1);
				src[1] = AsShaderResource(device, layers->at(i).right.texture, -1);
				mAADT.DrawAADT(resultRenderTarget2to1[index], &resultViewport, src, layers->at(i).left.bounds, layers->at(i).right.bounds, !(i == 0));
			}

		}
		else 
		{
			if (gLog)
			{
				int eye = 0;
				if (layers->at(i).left.texture == nullptr)
				{
					eye = 0;
				}
				else if (layers->at(i).right.texture == nullptr)
				{
					eye = 1;
				}
				RVR_LOG_A("meger has null  %d", eye);
			}
		}

		/*else if (layers->at(i).left.texture == nullptr && layers->at(i).right.texture != nullptr)
		{
			ID3D11ShaderResourceView* src[2];
			src[0] = AsShaderResource(device, layers->at(i).right.texture);
			src[1] = AsShaderResource(device, layers->at(i).right.texture);
			Blit2::Blit2to1(resultRenderTarget2to1[index], &resultViewport, src, layers->at(i).right.bounds, layers->at(i).right.bounds, !(i == 0));
		}
		else if (layers->at(i).left.texture != nullptr && layers->at(i).right.texture == nullptr)
		{
			ID3D11ShaderResourceView* src[2];
			src[0] = AsShaderResource(device, layers->at(i).left.texture);
			src[1] = AsShaderResource(device, layers->at(i).left.texture);		
			Blit2::Blit2to1(resultRenderTarget2to1[index], &resultViewport, src, layers->at(i).left.bounds, layers->at(i).left.bounds, !(i == 0));
		}*/

	}
	if (gPrintMsg)
	{
		if (index % 2 == 0)
		{
			Blit2::WirteText(L"pico test");
		}

	}
	
	Clear();
}

void Compositor::Resize()
{
	float backgroundColor[4] = { 8.05f, 0.05f, 0.09f, 1.0f };
		D3DHelper::Clear(context, resultResizeRenderTarget[0], backgroundColor);
		D3DHelper::Clear(context, resultResizeRenderTarget[1], backgroundColor);
		//D3DHelper::SaveTextureToFile(device, resultTexture[0], L"reszierorigin.jpg");
		Blit2::Resize(resultResizeRenderTarget[0], &resultResizeViewport, AsShaderResource(device, resultTexture[0]), mDepthStencilView[0]);
		Blit2::Resize(resultResizeRenderTarget[1], &resultResizeViewport, AsShaderResource(device, resultTexture[1]), mDepthStencilView[1]);

		//D3DHelper::SaveTextureToFile(device, resultResizeTexture[0], L"reszieleft.jpg");
 
}


//-----------------------------------------------------------------------------
ID3D11Texture2D* Compositor::Get(int index)
//-----------------------------------------------------------------------------
{
	ID3D11Texture2D* texture = nullptr;

	if ((index >= 0) && (index < 2))
	{
        texture = resultTexture[index];
	}
	return texture;
}

//-----------------------------------------------------------------------------
ID3D11Texture2D* Compositor::Get2to1(int index)
//-----------------------------------------------------------------------------
{
	ID3D11Texture2D* texture = nullptr;

	if ((index >= 0) && (index < 2))
	{
		texture = resultTexture2to1[index];
	}
	return texture;
}


//-----------------------------------------------------------------------------
ID3D11Texture2D* Compositor::GetNV(int index)
//-----------------------------------------------------------------------------
{
	ID3D11Texture2D* texture = nullptr;

	if ((index >= 0) && (index < 2))
	{
		texture = resultNVTexture[index];
	}
	return texture;
}

ID3D11Texture2D * Compositor::GetResize(int index)
{
	ID3D11Texture2D* texture = nullptr;

	if ((index >= 0) && (index < 2))
	{
		texture = resultResizeTexture[index];
	}
	return texture;
}

//-----------------------------------------------------------------------------
int Compositor::NumberOfLayers()
//-----------------------------------------------------------------------------
{
    int size = 0;
    if (layers != nullptr) size = layers->size();

    return size;
}

//-----------------------------------------------------------------------------
void Compositor::Clear()
//-----------------------------------------------------------------------------
{
	if (init_==false)
	{
		return;
	}
	layers->clear();
}

//-----------------------------------------------------------------------------
void Compositor::Destroy()
//-----------------------------------------------------------------------------
{
	if (init_==false)
	{
		return;
	}
	layers->clear();
	delete layers;
	for (int i = 0; i < 2; i++)
	{
		if (gConfigReader.BigPicture())
		{
			resultTexture2to1[i]->Release();
			resultRenderTarget2to1[i]->Release();
		} 
		else
		{
			resultTexture[i]->Release();
			resultRenderTarget[i]->Release();
		}
	
		
		//resultNVTexture[i]->Release();
	}

	DestroyResourceMap();
#ifdef picomode
	Blit2::DestroyBlit2Resources();
#else
	Blit::DestroyBlitResources();
#endif
	context->Release();
    device->Release();
}
ID3D11Texture2D* Compositor::GetSharedTexture(int i)
{
	return shared_texture_[i];
}
HANDLE Compositor::GetSharedTextureHandle(int i)
{
	return shared_texture_handle_[i];
}
ID3D11Texture2D* Compositor::GetSplitTextureForRender(int& index)
{
	int ret_index = 0;
	mutex_split_encoder_index.lock();
	while ((split_render_index% SplitTextureSize)==(split_encoder_index% SplitTextureSize))
	{
		split_render_index++;
		RVR_LOG_A("split_render_index add");
	}
	ret_index = split_render_index % SplitTextureSize;
	mutex_split_encoder_index.unlock();
	index = ret_index;
	return split_texture_[ret_index];
}
void Compositor::AddSplitRenderIndex() 
{
	mutex_split_encoder_index.lock();
	last_split_render_index = split_render_index;
	split_render_index++;
	mutex_split_encoder_index.unlock();
}
HANDLE Compositor::GetSplitTextureHandleForEncoder(int& index)
{
	while (last_split_render_index==-1)
	{
		Sleep(1);
	}
	mutex_split_encoder_index.lock();
	split_encoder_index = last_split_render_index;
	mutex_split_encoder_index.unlock();
	index = split_encoder_index % SplitTextureSize;
	return split_texture_handle_[split_encoder_index%SplitTextureSize];
}
//-----------------------------------------------------------------------------
void Compositor::DestroyResourceMap()
//-----------------------------------------------------------------------------
{
	auto it = srvMap.begin();
	while (it != srvMap.end())
	{
		it->second->Release();
		++it;
	}
}

//-----------------------------------------------------------------------------
ID3D11ShaderResourceView* Compositor::AsShaderResource(ID3D11Device* device, ID3D11Texture2D* texture, int texture2D_mipLevels)
//-----------------------------------------------------------------------------
{
	ID3D11ShaderResourceView* srv;
	if (srvMap.find(texture) != srvMap.end())
	{
		srv = srvMap[(HANDLE)texture];
	}
	else {
		D3D11_SHADER_RESOURCE_VIEW_DESC sRVDesc;
        ::ZeroMemory(&sRVDesc, sizeof(sRVDesc));
		D3DHelper::GetTextureInfo(texture, &sRVDesc.Format, nullptr, nullptr);
		sRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		sRVDesc.Texture2D.MipLevels = texture2D_mipLevels;
		
        if (!SUCCEEDED(device->CreateShaderResourceView(texture, &sRVDesc, &srv)))
        {
            DriverLog("Failed to Map SRV!!!!!!!!!!!!!!!!!!!!!!!!!!");
        }
        else {
            srvMap[texture] = srv;
        }
	}

	return srv;
}
//////////
//////////id3d11rendertargetview* compositor::asrendertarget(id3d11device* device, id3d11texture2d* texture)
//////////{
//////////    id3d11rendertargetview* rt;
//////////    if (rtmap.find(texture) != rtmap.end())
//////////    {
//////////        rt = rtmap[(handle)texture];
//////////    }
//////////    else {
//////////        if (!succeeded(device->createrendertargetview(texture, null, &rt)))
//////////        {
//////////            driverlog("failed to map srv!!!!!!!!!!!!!!!!!!!!!!!!!!");
//////////        }
//////////        else {
//////////            rtmap[texture] = rt;
//////////        }
//////////    }
//////////
//////////    return rt;
//////////}