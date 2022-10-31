//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------
#include <stdint.h>
#include "DepthResize.h"
#include "D3DHelper.h"
 
#include "HLSL/PixelShader.h"
#include "HLSL/VertexShader.h"
#include "HLSL/PixelResizeShader.h"
namespace DepthResize {

	struct QuadVertex {
		float x, y;
		float u, v;
		uint32_t view;
	};
	 
	ID3D11Buffer* mVB;
	ID3D11VertexShader* _vs;
	ID3D11PixelShader* ps_simpleCopy;
	ID3D11PixelShader* ps_simpleResize;
	ID3D11InputLayout* il;
	ID3D11ShaderResourceView* srv;
	ID3D11SamplerState* ss;
	ID3D11RasterizerState* rs;
	D3D11_VIEWPORT viewport;
	ID3D11BlendState* blendStateF;
	ID3D11BlendState* blendStateT;
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	ID3D11Buffer* mpConstantBuffer;
	ID3D11Buffer* mpAdjustMentBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	float mWidth;
	float mHeigth;
	void SetWH(float w, float y)
	{
		mWidth = w; mHeigth = y;
	};
	void UpdateVB(float* bounds)
	{
		float uMin, uMax, vMin, vMax;

		if (bounds != nullptr)
		{
			uMin = bounds[0];
			uMax = bounds[1];
			vMin = bounds[2];
			vMax = bounds[3];
		}
		else
		{
			uMin = 0.0f;
			uMax = 1.0f;
			vMin = 0.0f;
			vMax = 1.0f;
		}
		
		{
			QuadVertex v[6] = {
		   { -1, -1, uMin, vMax,0 },
		   { -1,  1, uMin, vMin,0  },
		   {  1, -1, uMax, vMax ,0 },
		   {  1,  1, uMax, vMin ,0 },
			};

			deviceContext->UpdateSubresource(mVB, 0, NULL, v, sizeof(QuadVertex) * 4, 0);
		}

	}

	 

	void DepthResizeFun(  ID3D11RenderTargetView* dst,   D3D11_VIEWPORT* dstViewport,   ID3D11ShaderResourceView* src )
	{
		float backgroundColor[4] = { 0.00f, 0.00f, 0.00f, 0.0f };

		D3DHelper::Clear(deviceContext, dst, backgroundColor);
	 

		QuadVertex v[6] = {
			{ -1, -1, 0, 1 },
			{ -1,  1, 0, 0  },
			{  1, -1, 1, 1  },
			{  1,  1, 1, 0  },
		};
		deviceContext->UpdateSubresource(mVB, 0, NULL, v, sizeof(QuadVertex) * 4, 0);
		UINT stride = sizeof(QuadVertex);
		UINT offset = 0;
		deviceContext->ClearState();
		{ //src
			deviceContext->PSSetSamplers(0, 1, &ss);
			deviceContext->PSSetShaderResources(0, 1, &src);
		}
		{ //dst
			deviceContext->OMSetRenderTargets(1, &dst, nullptr);
			deviceContext->RSSetViewports(1, dstViewport);
		}

		{ //quad

			deviceContext->IASetInputLayout(il);
			deviceContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
			deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		}


		/*{
			deviceContext->OMSetBlendState(blendStateT, 0, 0xffffffff);
		}*/


		deviceContext->VSSetShader(_vs, nullptr, 0);
		deviceContext->PSSetShader(ps_simpleResize, nullptr, 0);

		deviceContext->Draw(4, 0);
		deviceContext->Flush();

	}
	void CreateDepthResizeResources(ID3D11Device* device)
	{

		device = device;
		device->GetImmediateContext(&deviceContext);
		 
		{
			D3D11_RASTERIZER_DESC rsDesc;
			::ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
			rsDesc.CullMode = D3D11_CULL_NONE;
			rsDesc.ScissorEnable = FALSE;
			rsDesc.MultisampleEnable = FALSE;
			rsDesc.AntialiasedLineEnable = FALSE;
			rsDesc.DepthClipEnable = FALSE;
			rsDesc.FillMode = D3D11_FILL_SOLID;
			rsDesc.FrontCounterClockwise = FALSE;
			rsDesc.DepthBias = 0;
			rsDesc.DepthBiasClamp = 0.0f;
			rsDesc.SlopeScaledDepthBias = 0.0f;
			device->CreateRasterizerState(&rsDesc, &rs);
		}
		{
			D3D11_BLEND_DESC blendDesc;
			blendDesc.AlphaToCoverageEnable = true;
			blendDesc.IndependentBlendEnable = false;
			blendDesc.RenderTarget[0].BlendEnable = false;
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;;
			device->CreateBlendState(&blendDesc, &blendStateF);
		}
		{
			D3D11_BLEND_DESC blendDesc;
			blendDesc.AlphaToCoverageEnable = true;
			blendDesc.IndependentBlendEnable = false;
			blendDesc.RenderTarget[0].BlendEnable = true;
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			device->CreateBlendState(&blendDesc, &blendStateT);
		}
		{

			device->CreateVertexShader(g_VertexShader, sizeof(g_VertexShader), nullptr, &_vs);
			//device->CreatePixelShader(g_PixelShader, sizeof(g_PixelShader), nullptr, &ps_simpleCopy);
			device->CreatePixelShader(g_PixelResizeShader, sizeof(g_PixelResizeShader), nullptr, &ps_simpleResize);
		}
		{
			D3D11_SAMPLER_DESC samplerDesc;
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MipLODBias = 0.0f;
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			samplerDesc.BorderColor[0] = 0;
			samplerDesc.BorderColor[1] = 0;
			samplerDesc.BorderColor[2] = 0;
			samplerDesc.BorderColor[3] = 0;
			samplerDesc.MinLOD = 0;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
			device->CreateSamplerState(&samplerDesc, &ss);
		}
		{
			D3D11_INPUT_ELEMENT_DESC s_DX11InputElementDesc[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				 { "VIEW", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			UINT numElements = ARRAYSIZE(s_DX11InputElementDesc);

			device->CreateInputLayout(s_DX11InputElementDesc, numElements, g_VertexShader, sizeof(g_VertexShader), &il);
		}
		{
			D3D11_BUFFER_DESC bufferDesc = {};
			::memset(&bufferDesc, 0, sizeof(bufferDesc));
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			 
			bufferDesc.ByteWidth = sizeof(QuadVertex) * 6;
			

			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			device->CreateBuffer(&bufferDesc, NULL, &mVB);
			
		}

	}

	void DestroyBlit2Resources()
	{
		rs->Release();
		blendStateF->Release();
		blendStateT->Release();
		_vs->Release();
		ps_simpleCopy->Release();
		ss->Release();
		il->Release();
		mVB->Release();
		deviceContext->Release();
	}
}