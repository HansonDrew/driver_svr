#pragma once

#include "d3d11.h"
#include <vector>
#include <iostream>

#include "distortion_dp.h"
#include "config_reader.h"

#include "driverlog.h"

//using namespace DirectX;
 

class AADT_Func
{
	struct QuadVertex 
	{
		float x, y;
		float u, v;
		uint32_t view;
	};

	struct VSConstantBuffer
	{

		float x;
		float y;
		float cutx;
		float cuty;
	};
	
	struct PSAdjustmentBuffer
	{

		float bright;
		float saturation;
		float contrast;
		float gamma;
		float a;
		float b;
		float c;
		int function;
	};

	struct VSConstantBuffer_Bounds
	{

		float ulMin;
		float ulMax;
		float vlMin;
		float vlMax;

		float urMin;
		float urMax;
		float vrMin;
		float vrMax;
	};


	struct VSConstantBuffer_Bounds_test
	{

		float test;
		float test2;
		float test3;
		float test4;
		
	};


public:
	
	
	void CreateAADTResources(ID3D11Device* d3d11Device, int w, int h ,bool AADTFlag,int compress, int origin);
	void DrawAADT(ID3D11RenderTargetView* dst, D3D11_VIEWPORT* dstViewport, ID3D11ShaderResourceView* src[2], float* leftbounds, float* rightbounds, bool blendingEnabled);



private:

	bool AdaptiveCompressionFlag = false;
	float AdaptiveCompressionRate;

	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;

	//CB
	ID3D11Buffer* mpConstantBuffer;
	ID3D11Buffer* mpAdjustMentBuffer;
	ID3D11Buffer* mConstantBuffer_Bounds;

	ID3D11Buffer* mConstantBuffer_Bounds_test;

	ID3D11SamplerState* mSamplerState;
	ID3D11RasterizerState* mRasterizerState;

	ID3D11BlendState* blendStateF;
	ID3D11BlendState* blendStateT;

	ID3D11VertexShader* _vs;
	ID3D11PixelShader* ps_simpleCopy;
	ID3D11PixelShader* ps_simpleResize;

	ID3D11InputLayout* mInputLayout;

	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;

	float which_ndc_corrspond_tex1=0.912f;

	float compress_border = 0.5472;

	float AdaptiveBorder;
	float OriginBorder;
	//int zoom_border = 15;
	int zoom_border = 15;

	



	int MeshSize_width;
	int MeshSize_height;
	int half_MeshSize_width;
	int half_MeshSize_height;
	std::vector<float> m_Recorder_data;

	std::vector<QuadVertex> m_vertices;
	std::vector<UINT> m_indices;

	std::vector<QuadVertex> m_vertices_test;
	std::vector<UINT> m_indices_test;

	UINT m_IndexCount;
	UINT m_VertexCount;
	UINT m_VertexStride;

	

	void CreateNDCborder();
	void CreateMesh();
	void CreateMesh_Test();
	

};

