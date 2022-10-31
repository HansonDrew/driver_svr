//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#pragma once
#include<d3d11.h>
#include<vector>
#include<map>
#include <mutex>
#include "AADT_Func.h"
//#define PICO_PRESENT_MODE
#define SplitTextureSize 4
#define picomode
class Compositor {
public:
	class LayerDesc {
	public:
		class EyeInfo {
		public:
			ID3D11Texture2D* texture;
			float bounds[4];
		public:
			EyeInfo()
			{
				texture = nullptr;
			}
		};
		EyeInfo left;
		EyeInfo right;
	};
	AADT_Func mAADT;
	void SetQ1Q2(float q1[4], float q2[4]);
	bool Initialize(ID3D11Device* device, int w, int h);
	void Add(LayerDesc* layerDesc);
	void Merge();
	void Merge2to1(int index);
	void Resize();
#ifdef PICO_PRESENT_MODE
	void Save();
	ID3D11Texture2D* GetEncode(int i);
#endif
	void Clear();
	void Destroy();
    int NumberOfLayers();
	ID3D11Texture2D* Get(int i);
	ID3D11Texture2D* Get2to1(int i);
	ID3D11Texture2D * GetNV(int index);
	ID3D11Texture2D * GetResize(int index);
	ID3D11Texture2D* GetSharedTexture(int i);
	HANDLE GetSharedTextureHandle(int i);

	ID3D11Texture2D* GetSplitTextureForRender(int &index);
	void AddSplitRenderIndex();
	HANDLE GetSplitTextureHandleForEncoder(int& index);
private:
	bool init_ = false;
	HANDLE shared_texture_handle_[2];
	ID3D11Texture2D* shared_texture_[2];

	HANDLE split_texture_handle_[SplitTextureSize];
	ID3D11Texture2D* split_texture_[SplitTextureSize];
	int last_split_render_index = -1;
	int split_render_index = 0;
	int split_encoder_index = -1;
	std::mutex mutex_split_encoder_index;
	
	ID3D11ShaderResourceView* AsShaderResource(ID3D11Device* device, ID3D11Texture2D* texture, int texture2D_mipLevels = 1);
	void DestroyResourceMap();
private:
	D3D11_VIEWPORT resultViewport;
	ID3D11Texture2D * mDepthStencil[2];
	ID3D11DepthStencilView * mDepthStencilView[2];
	ID3D11Texture2D* resultTexture[2];
	HANDLE resultTextureHandle[2];
	ID3D11Texture2D* resultNVTexture[2];
	ID3D11Texture2D* resultResizeTexture[2];
	D3D11_VIEWPORT resultResizeViewport;
	ID3D11Texture2D* resultTexture2to1[2];
#ifdef PICO_PRESENT_MODE
	ID3D11Texture2D* encodeTexture[2];
#endif
	ID3D11RenderTargetView* resultRenderTarget[2];
	ID3D11RenderTargetView* resultRenderTarget2to1[2];
	ID3D11RenderTargetView* resultResizeRenderTarget[2];

	std::vector<LayerDesc>* layers;
	
	ID3D11Device* device;
	ID3D11DeviceContext* context;

	std::map<HANDLE, ID3D11ShaderResourceView*> srvMap;
public:
    int Width, Height;
};
