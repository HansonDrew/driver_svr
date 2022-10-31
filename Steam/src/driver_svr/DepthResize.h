//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#pragma once
#include "d3d11.h"
namespace DepthResize {
	 
	void DepthResizeFun( ID3D11RenderTargetView* dst,  D3D11_VIEWPORT* dstViewport,  ID3D11ShaderResourceView* src );
	void SetWH(float w, float y);
	void CreateDepthResizeResources(ID3D11Device* d3d11Device);
	void DestroyBlit2Resources();
}
