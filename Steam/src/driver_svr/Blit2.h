//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#pragma once
#include "d3d11.h"
#define INDEXNUM 54
#define VERTEXNUM 16
#define INDEXNUM2to1 108
#define VERTEXNUM2to1 36
enum class  color_flag
{
	white =0,
	red
};
namespace Blit2 {
	void Blit2(ID3D11RenderTargetView* dst, D3D11_VIEWPORT* dstViewport, ID3D11ShaderResourceView* src, ID3D11DepthStencilView*depsrc, float* bounds, bool);
	void Blit2to1(ID3D11RenderTargetView* dst, D3D11_VIEWPORT* dstViewport, ID3D11ShaderResourceView*src[2], float* leftbounds, float* rightbounds, bool blendingEnabled);
	void Resize(ID3D11RenderTargetView* dst, D3D11_VIEWPORT* dstViewport, ID3D11ShaderResourceView* src, ID3D11DepthStencilView*depsrc);
	//void DepthResize(ID3D11RenderTargetView* dst, D3D11_VIEWPORT* dstViewport, ID3D11ShaderResourceView* src, ID3D11DepthStencilView* depsrc);
	void SetWH(float w, float y);
	void CreateBlit2Resources(ID3D11Device* d3d11Device);
	void CreateTextResources(ID3D11Texture2D* d3d11Texture);
	void WirteText(WCHAR* text,color_flag);
	void DestroyBlit2Resources();
	void SetQ1Q2(float q1[4], float q2[4]);
}
