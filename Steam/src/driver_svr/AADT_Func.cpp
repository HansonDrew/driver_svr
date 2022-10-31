#include "AADT_Func.h"

#include "HLSL/PixelShader.h"
#include "HLSL/AAdtVertexShader.h"
#include "HLSL/PixelResizeShader.h"

extern ConfigReader gConfigReader;

void AADT_Func::CreateNDCborder()
{
	
	float red_output[2] = { 0.0f, 0.0f };
	float green_output[2] = { 0.0f, 0.0f };
	float bule_output[2] = { 0.0f, 0.0f };

	float last = 0.0f;
	float test_input[2] = { 0.0f,0.0f };


	int i = 0;
	while (i < 1001)
	{


		WarpTexCoordChromaModeNdc(test_input, red_output, green_output, bule_output);

		//std::cout << "[0720][AADT][Distortion][green]index:" << i << std::endl;

		//std::cout << "[0720][AADT][Distortion][green][in]r:" << test_input[0] << std::endl;

		//std::cout << "[0720][AADT][Distortion][green][out]R:" << green_output[0] << std::endl;		

		//std::cout << "[0720][AADT][Distortion][green][delta]:" << green_output[0] - last << std::endl;


		if (green_output[0] > 1.0f && last <= 1.0f)
		{
			which_ndc_corrspond_tex1 = float(i) / 1000.0f;
		}


		last = green_output[0];
		std::cout << " " << std::endl;


		i++;
		

		test_input[0] = test_input[0] + 0.001f;
	}



}
void AADT_Func::CreateMesh()
{
	float red_output[2] = { 0.0f, 0.0f };
	float green_output[2] = { 0.0f, 0.0f };
	float bule_output[2] = { 0.0f, 0.0f };
	float test_input[2] = { 0.0f,0.0f };

	float ndc_gap = which_ndc_corrspond_tex1 / float(half_MeshSize_width);

	int i = 0;
	while (i < half_MeshSize_width+1)
	{

		WarpTexCoordChromaModeNdc(test_input, red_output, green_output, bule_output);
		m_Recorder_data.push_back(green_output[0]);

		i++;
		test_input[0] = test_input[0] + ndc_gap;
	}




	int already_push = 0;	
	QuadVertex a_point;
	float ndc_uniform_gap = 1.0f / float(half_MeshSize_width);


	OriginBorder = ndc_uniform_gap * float(zoom_border);
	
	

	for (int view = 0; view < 2; view++)
	{
	//------------------------------
	//Right-UP
	//------------------------------
		for (int n = 0; n <= half_MeshSize_height; n++)
		{
			for (int m = 0; m <= half_MeshSize_width; m++)
			{
				float x, y, z;
				float u, v;



				x = float(m) * ndc_uniform_gap;
				y = 1.0 - float(n) * ndc_uniform_gap;
                z = 0.5f;




				if (AdaptiveCompressionFlag == true)
				{
					if (m < zoom_border+1)
					{
						x = float(m) * (AdaptiveBorder / OriginBorder)* ndc_uniform_gap;
					}
					else
					{
						x = AdaptiveBorder + float(m - zoom_border) * ((1.0f- AdaptiveBorder) /(1.0f- OriginBorder) )* ndc_uniform_gap;
					}


					if (n <= (half_MeshSize_width - zoom_border))
					{
						y = 1.0f - float(n) * ((1.0f - AdaptiveBorder) / (1.0f - OriginBorder))* ndc_uniform_gap;
					}
					else
					{
						y = 1.0f - (1.0f - AdaptiveBorder) -float(n- (half_MeshSize_width - zoom_border))* (AdaptiveBorder / OriginBorder)* ndc_uniform_gap;
					}

				}




				

				if (m < zoom_border+1)
				{
					u = m * m_Recorder_data[zoom_border] / float(zoom_border);
				}
				else
				{
					u = m_Recorder_data[m];
				}
				u = u * 0.5f + 0.5f;


				if (n > (half_MeshSize_width- zoom_border))
				{
					v = (half_MeshSize_width - n) * m_Recorder_data[zoom_border] / float(zoom_border);
				}
				else
				{
					v = m_Recorder_data[half_MeshSize_width - n];
				}

				v = 0.5f - v * 0.5f;





				if (n == 0)
				{
					DriverLog("[0812][DrawAADT][CreateMesh]m=%d, x=%f, u=%f  u(in hmd)%f" ,m,x,u  ,x*0.25f+0.25f );
				}



				a_point.x = x;
				a_point.y = y;
				a_point.u = u;
				a_point.v = v;
				a_point.view = view;

				m_vertices.push_back(a_point);
			}
		}
		//m*n
		for (int n = 0; n < half_MeshSize_height; n++)
		{
			for (int m = 0; m < half_MeshSize_width; m++)
			{
				int left_up = already_push + (half_MeshSize_width + 1) * n + m;
				int right_up = already_push + (half_MeshSize_width + 1) * n + m + 1;
				int left_down = already_push + (half_MeshSize_width + 1) * (n + 1) + m;
				int right_down = already_push + (half_MeshSize_width + 1) * (n + 1) + m + 1;

				m_indices.push_back(left_down);
				m_indices.push_back(left_up);
				m_indices.push_back(right_up);
				m_indices.push_back(right_up);
				m_indices.push_back(right_down);
				m_indices.push_back(left_down);

			}
		}

		already_push = m_vertices.size();



		//------------------------------
		//Right-Down
		//------------------------------

		for (int n = 0; n <= half_MeshSize_height; n++)
		{
			for (int m = 0; m <= half_MeshSize_width; m++)
			{
				float x, y, z;
				float u, v;


				x = float(m) * ndc_uniform_gap;
				y = -float(n) * ndc_uniform_gap;
				z = 0.5f;



				if (AdaptiveCompressionFlag == true)
				{
					if (m < zoom_border + 1)
					{
						x = float(m) * (AdaptiveBorder / OriginBorder)* ndc_uniform_gap;
					}
					else
					{
						x = AdaptiveBorder + float(m - zoom_border) * ((1.0f - AdaptiveBorder) / (1.0f - OriginBorder))* ndc_uniform_gap;
					}


					if (n <= zoom_border)
					{
						y =  - float(n) * (AdaptiveBorder / OriginBorder)* ndc_uniform_gap;
					}
					else
					{
						y = - AdaptiveBorder - float(n  - zoom_border) * ((1.0f - AdaptiveBorder) / (1.0f - OriginBorder))* ndc_uniform_gap;
					}

				}





				if (m < (zoom_border+1))
				{
					u = m * m_Recorder_data[zoom_border] / float(zoom_border);
				}
				else
				{
					u = m_Recorder_data[m];;

				}
				u = u * 0.5f + 0.5f;


				if (n < (zoom_border + 1))
				{
					v = n * m_Recorder_data[zoom_border] / float(zoom_border);


				}
				else
				{
					v = m_Recorder_data[n];

				}
				v = v * 0.5f + 0.5f;


				a_point.x = x;
				a_point.y = y;
				a_point.u = u;
				a_point.v = v;
				a_point.view = view;

				m_vertices.push_back(a_point);
			}
		}
		//m*n
		for (int n = 0; n < half_MeshSize_height; n++)
		{
			for (int m = 0; m < half_MeshSize_width; m++)
			{
				int left_up = already_push + (half_MeshSize_width + 1) * n + m;
				int right_up = already_push + (half_MeshSize_width + 1) * n + m + 1;
				int left_down = already_push + (half_MeshSize_width + 1) * (n + 1) + m;
				int right_down = already_push + (half_MeshSize_width + 1) * (n + 1) + m + 1;

				m_indices.push_back(left_down);
				m_indices.push_back(left_up);
				m_indices.push_back(right_up);
				m_indices.push_back(right_up);
				m_indices.push_back(right_down);
				m_indices.push_back(left_down);

			}
		}

		already_push = m_vertices.size();


		//------------------------------
		//Left-UP
		//------------------------------

		for (int n = 0; n <= half_MeshSize_height; n++)
		{
			for (int m = 0; m <= half_MeshSize_width; m++)
			{
				float x, y, z;
				float u, v;


				x = -1.0f + float(m) * ndc_uniform_gap;
				y = 1.0f - float(n) * ndc_uniform_gap;
				z = 0.5f;



				if (AdaptiveCompressionFlag == true)
				{
					if (m <= (half_MeshSize_width - zoom_border))
					{
						x = -1.0f + float(m) * ((1.0f - AdaptiveBorder) / (1.0f - OriginBorder))* ndc_uniform_gap;
					}
					else
					{
						x = -1.0f + (1.0f - AdaptiveBorder) + float(m - (half_MeshSize_width - zoom_border)) * (AdaptiveBorder / OriginBorder)* ndc_uniform_gap;
					}


					if (n <= (half_MeshSize_width - zoom_border))
					{
						y = 1.0f - float(n) * ((1.0f - AdaptiveBorder) / (1.0f - OriginBorder))* ndc_uniform_gap;
					}
					else
					{
						y = 1.0f - (1.0f - AdaptiveBorder) - float(n - (half_MeshSize_width - zoom_border)) * (AdaptiveBorder / OriginBorder)* ndc_uniform_gap;
					}

				}





				if (m > (half_MeshSize_width-zoom_border))
				{
					u = (half_MeshSize_width - m) * m_Recorder_data[zoom_border] / float(zoom_border);
				}
				else
				{
					u = m_Recorder_data[half_MeshSize_width - m];

				}
				u = -u * 0.5f + 0.5f;


				if (n > (half_MeshSize_width - zoom_border))
				{
					v = (half_MeshSize_width - n) * m_Recorder_data[zoom_border] / float(zoom_border);
				}
				else
				{
					v = m_Recorder_data[half_MeshSize_width - n];
				}

				v = 0.5f - v * 0.5f;


				a_point.x = x;
				a_point.y = y;
				a_point.u = u;
				a_point.v = v;
				a_point.view = view;

				m_vertices.push_back(a_point);
			}
		}
		//m*n
		for (int n = 0; n < half_MeshSize_height; n++)
		{
			for (int m = 0; m < half_MeshSize_width; m++)
			{
				int left_up = already_push + (half_MeshSize_width + 1) * n + m;
				int right_up = already_push + (half_MeshSize_width + 1) * n + m + 1;
				int left_down = already_push + (half_MeshSize_width + 1) * (n + 1) + m;
				int right_down = already_push + (half_MeshSize_width + 1) * (n + 1) + m + 1;

				m_indices.push_back(left_down);
				m_indices.push_back(left_up);
				m_indices.push_back(right_up);
				m_indices.push_back(right_up);
				m_indices.push_back(right_down);
				m_indices.push_back(left_down);

			}
		}

		already_push = m_vertices.size();



		//------------------------------
		//Left-Down
		//------------------------------

		for (int n = 0; n <= half_MeshSize_height; n++)
		{
			for (int m = 0; m <= half_MeshSize_width; m++)
			{
				float x, y, z;
				float u, v;


				x = -1.0f + float(m) * ndc_uniform_gap;
				y = -float(n) * ndc_uniform_gap;
				z = 0.5f;




				if (AdaptiveCompressionFlag == true)
				{
					if (m <= (half_MeshSize_width - zoom_border))
					{
						x = -1.0f + float(m) * ((1.0f - AdaptiveBorder) / (1.0f - OriginBorder))* ndc_uniform_gap;
					}
					else
					{
						x = -1.0f + (1.0f - AdaptiveBorder) + float(m - (half_MeshSize_width - zoom_border)) * (AdaptiveBorder / OriginBorder)* ndc_uniform_gap;
					}


					if (n <= zoom_border)
					{
						y = -float(n) * (AdaptiveBorder / OriginBorder)* ndc_uniform_gap;
					}
					else
					{
						y = -AdaptiveBorder - float(n - zoom_border) * ((1.0f - AdaptiveBorder) / (1.0f - OriginBorder))* ndc_uniform_gap;
					}

				}





				if (m > (half_MeshSize_width - zoom_border))
				{
					u = (half_MeshSize_width - m) * m_Recorder_data[zoom_border] / float(zoom_border);
				}
				else
				{
					u = m_Recorder_data[half_MeshSize_width - m];

				}
				u = -u * 0.5f + 0.5f;


				if (n < (zoom_border+1))
				{
					v = n * m_Recorder_data[zoom_border] / float(zoom_border);


				}
				else
				{
					v = m_Recorder_data[n];

				}
				v = v * 0.5f + 0.5f;


				a_point.x = x;
				a_point.y = y;
				a_point.u = u;
				a_point.v = v;
				a_point.view = view;

				m_vertices.push_back(a_point);
			}
		}
		//m*n
		for (int n = 0; n < half_MeshSize_height; n++)
		{
			for (int m = 0; m < half_MeshSize_width; m++)
			{
				int left_up = already_push + (half_MeshSize_width + 1) * n + m;
				int right_up = already_push + (half_MeshSize_width + 1) * n + m + 1;
				int left_down = already_push + (half_MeshSize_width + 1) * (n + 1) + m;
				int right_down = already_push + (half_MeshSize_width + 1) * (n + 1) + m + 1;

				m_indices.push_back(left_down);
				m_indices.push_back(left_up);
				m_indices.push_back(right_up);
				m_indices.push_back(right_up);
				m_indices.push_back(right_down);
				m_indices.push_back(left_down);

			}
		}

		already_push = m_vertices.size();

	}

	m_IndexCount = m_indices.size();
	m_VertexStride = sizeof(QuadVertex);
	m_VertexCount = m_vertices.size();



}


void AADT_Func::CreateMesh_Test()
{
	QuadVertex a_point;

	a_point.x = -1.0f;
	a_point.y = 1.0f;
	a_point.u = 0.0f;
	a_point.v = 0.0f;
	a_point.view = 0;

	m_vertices.push_back(a_point);

	a_point.x = 1.0f;
	a_point.y = 1.0f;
	a_point.u = 1.0f;
	a_point.v = 0.0f;
	a_point.view = 0;

	m_vertices.push_back(a_point);

	a_point.x = 1.0f;
	a_point.y = -1.0f;
	a_point.u = 1.0f;
	a_point.v = 1.0f;
	a_point.view = 0;

	m_vertices.push_back(a_point);

	a_point.x = -1.0f;
	a_point.y = -1.0f;
	a_point.u = 0.0f;
	a_point.v = 1.0f;
	a_point.view = 0;

	m_vertices.push_back(a_point);





	a_point.x = -1.0f;
	a_point.y = 1.0f;
	a_point.u = 0.0f;
	a_point.v = 0.0f;
	a_point.view = 1;

	m_vertices.push_back(a_point);

	a_point.x = 1.0f;
	a_point.y = 1.0f;
	a_point.u = 1.0f;
	a_point.v = 0.0f;
	a_point.view = 1;

	m_vertices.push_back(a_point);

	a_point.x = 1.0f;
	a_point.y = -1.0f;
	a_point.u = 1.0f;
	a_point.v = 1.0f;
	a_point.view = 1;

	m_vertices.push_back(a_point);

	a_point.x = -1.0f;
	a_point.y = -1.0f;
	a_point.u = 0.0f;
	a_point.v = 1.0f;
	a_point.view = 1;

	m_vertices.push_back(a_point);


	m_indices.push_back(0);
	m_indices.push_back(1);
	m_indices.push_back(2);

	m_indices.push_back(0);
	m_indices.push_back(2);
	m_indices.push_back(3);

	m_indices.push_back(4);
	m_indices.push_back(5);
	m_indices.push_back(6);

	m_indices.push_back(4);
	m_indices.push_back(6);
	m_indices.push_back(7);

	m_IndexCount = m_indices.size();
	
	m_VertexCount = m_vertices.size();


}


void AADT_Func::CreateAADTResources(ID3D11Device* d3d11Device,int w ,int h,bool AADTFlag,int compress,int origin)
{

	AdaptiveCompressionRate = float(compress)/float(origin);
	

	AdaptiveCompressionFlag = AADTFlag;

	AdaptiveBorder = compress_border / AdaptiveCompressionRate;

	//DriverLog("[0811][DrawAADT][CreateAADTResources] AdaptiveCompressionRate:%f compress:%d origin:%d \n ", AdaptiveCompressionRate, compress, origin);

	//DriverLog("[0811][DrawAADT][CreateAADTResources] AdaptiveBorder:%f \n ", AdaptiveBorder);


	device = d3d11Device;
	device->GetImmediateContext(&deviceContext);

	MeshSize_width = w;
	MeshSize_height = h;
	half_MeshSize_width = w / 2;
	half_MeshSize_height = h / 2;

	CreateMesh();

	//CreateMesh_Test();

	//CB
	{
		D3D11_BUFFER_DESC cbd;
		ZeroMemory(&cbd, sizeof(cbd));
		cbd.Usage = D3D11_USAGE_DEFAULT; //D3D11_USAGE_DYNAMIC;
		cbd.ByteWidth = sizeof(VSConstantBuffer);
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = 0;// D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0;
		cbd.StructureByteStride = 0;
		device->CreateBuffer(&cbd, NULL, &mpConstantBuffer);

		cbd.ByteWidth = sizeof(PSAdjustmentBuffer);
		device->CreateBuffer(&cbd, NULL, &mpAdjustMentBuffer);


		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags =  D3D11_CPU_ACCESS_WRITE;
		cbd.ByteWidth = sizeof(VSConstantBuffer_Bounds);
		device->CreateBuffer(&cbd, NULL, &mConstantBuffer_Bounds);

		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.ByteWidth = sizeof(VSConstantBuffer_Bounds_test);
		device->CreateBuffer(&cbd, NULL, &mConstantBuffer_Bounds_test);
	}
	//RS
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
		device->CreateRasterizerState(&rsDesc, &mRasterizerState);
	}
	//ALPHA BLEND
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
	//SHADER
	{

		device->CreateVertexShader(g_AAdtVertexShader, sizeof(g_AAdtVertexShader), nullptr, &_vs);
		device->CreatePixelShader(g_PixelShader, sizeof(g_PixelShader), nullptr, &ps_simpleCopy);
		device->CreatePixelShader(g_PixelResizeShader, sizeof(g_PixelResizeShader), nullptr, &ps_simpleResize);
	}
	//SAMPLE
	{
		D3D11_SAMPLER_DESC samplerDesc;
		//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 16;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		device->CreateSamplerState(&samplerDesc, &mSamplerState);
	}
	//INPUT LAYOUT
	{
		D3D11_INPUT_ELEMENT_DESC s_DX11InputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "VIEW", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(s_DX11InputElementDesc);
		device->CreateInputLayout(s_DX11InputElementDesc, numElements, g_AAdtVertexShader, sizeof(g_AAdtVertexShader), &mInputLayout);
	}

	
	{
		//VB
		D3D11_BUFFER_DESC bufferDesc = {};
		::memset(&bufferDesc, 0, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		if (gConfigReader.GetCutFlag() == 1)
		{
			if (gConfigReader.BigPicture())
			{
				bufferDesc.ByteWidth = sizeof(QuadVertex) * m_VertexCount;
			}
			else
			{
				bufferDesc.ByteWidth = sizeof(QuadVertex) * m_VertexCount/2;
			}
		}
		else
		{
			bufferDesc.ByteWidth = sizeof(QuadVertex) * 6;
		}
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &m_vertices[0];

		

		device->CreateBuffer(&bufferDesc, &initData, &mVB);

		//IB
		if (gConfigReader.GetCutFlag() == 1)
		{
			D3D11_BUFFER_DESC ibd;
			ibd.Usage = D3D11_USAGE_IMMUTABLE;
			ibd.ByteWidth = sizeof(UINT) * m_indices.size();
			ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			ibd.CPUAccessFlags = 0;
			ibd.MiscFlags = 0;

			initData.pSysMem = &m_indices[0];

			

			device->CreateBuffer(&ibd, &initData, &mIB);
		}

	}

}
void AADT_Func::DrawAADT(ID3D11RenderTargetView* dst, D3D11_VIEWPORT* dstViewport, ID3D11ShaderResourceView* src[2], float* leftbounds, float* rightbounds, bool blendingEnabled)
{
	UINT stride = sizeof(QuadVertex);
	UINT offset = 0;


	deviceContext->GenerateMips(src[0]);
	deviceContext->GenerateMips(src[1]);
	DriverLog("[0814][DrawAADT][AADT_Func::DrawAADT][GenerateMips]");

	deviceContext->ClearState();
	{ //src
		deviceContext->PSSetSamplers(0, 1, &mSamplerState);
		deviceContext->PSSetShaderResources(0, 2, src);
	}
	{ //dst
		deviceContext->OMSetRenderTargets(1, &dst, nullptr);
		deviceContext->RSSetViewports(1, dstViewport);
	}




	VSConstantBuffer_Bounds cb_bounds;


	if (leftbounds != nullptr)
	{
		cb_bounds.ulMin = leftbounds[0];
		cb_bounds.ulMax = leftbounds[1];
		cb_bounds.vlMin = leftbounds[2];
		cb_bounds.vlMax = leftbounds[3];
	}
	else
	{
		cb_bounds.ulMin = 0.0f;
		cb_bounds.ulMax = 1.0f;
		cb_bounds.vlMin = 0.0f;
		cb_bounds.vlMax = 1.0f;
	}

	if (rightbounds != nullptr)
	{
		cb_bounds.urMin = rightbounds[0];
		cb_bounds.urMax = rightbounds[1];
		cb_bounds.vrMin = rightbounds[2];
		cb_bounds.vrMax = rightbounds[3];
	}
	else
	{
		cb_bounds.urMin = 0.0f;
		cb_bounds.urMax = 1.0f;
		cb_bounds.vrMin = 0.0f;
		cb_bounds.vrMax = 1.0f;
	}

	/*cb_bounds.ulMin = 0.0f;
	cb_bounds.ulMax = 2.0f;
	cb_bounds.vlMin = 0.0f;
	cb_bounds.vlMax = 2.0f;

	cb_bounds.urMin = 0.0f;
	cb_bounds.urMax = 2.0f;
	cb_bounds.vrMin = 0.0f;
	cb_bounds.vrMax = 2.0f;*/
	
	/*cout << "[0806][DrawAADT]"  << endl;
	cout << "cb_bounds.ulMin" << cb_bounds.ulMin << endl;
	cout << "cb_bounds.vlMin" << cb_bounds.vlMin << endl;
	cout << "cb_bounds.ulMax" << cb_bounds.ulMax << endl;
	cout << "cb_bounds.vlMax" << cb_bounds.vlMax << endl;*/

	

	if (cb_bounds.ulMax < 0.999f)
	{
		DriverLog("[0806][DrawAADT][Special] ulMin %f vlMin %f ulMax %f vlMax %f src[2] %p %p \n ", cb_bounds.ulMin, cb_bounds.vlMin, cb_bounds.ulMax, cb_bounds.vlMax, src[0], src[1]);
	}
	if (cb_bounds.ulMax > 1.001f|| cb_bounds.vlMax > 1.001f)
	{
		DriverLog("[0806][DrawAADT][Special2] ulMin %f vlMin %f ulMax %f vlMax %f src[2] %p %p \n ", cb_bounds.ulMin, cb_bounds.vlMin, cb_bounds.ulMax, cb_bounds.vlMax, src[0], src[1]);
	}
	


	
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HRESULT hr = deviceContext->Map(mConstantBuffer_Bounds, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	memcpy_s(mappedData.pData, sizeof(VSConstantBuffer_Bounds), &cb_bounds, sizeof(VSConstantBuffer_Bounds));
	deviceContext->Unmap(mConstantBuffer_Bounds, 0);
	

	/*VSConstantBuffer_Bounds_test  test ;

	test.test = cb_bounds.ulMin;
	test.test2 = cb_bounds.ulMax;
	test.test3 = cb_bounds.vlMin;
	test.test4 = cb_bounds.vlMax;

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HRESULT hr = deviceContext->Map(mConstantBuffer_Bounds_test, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	memcpy_s(mappedData.pData, sizeof(VSConstantBuffer_Bounds_test), &test, sizeof(VSConstantBuffer_Bounds_test));
	deviceContext->Unmap(mConstantBuffer_Bounds_test, 0);*/



	{ //quad
		//UpdateVB2to1(leftbounds, rightbounds);
		deviceContext->IASetInputLayout(mInputLayout);
		deviceContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
		if (gConfigReader.GetCutFlag() == 1)
		{
			deviceContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);
			deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
		else
		{
			deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		}

	}

	if (blendingEnabled)
	{
		deviceContext->OMSetBlendState(blendStateT, 0, 0xffffffff);
	}

	deviceContext->RSSetState(mRasterizerState);
	deviceContext->VSSetShader(_vs, nullptr, 0);

	//deviceContext->UpdateSubresource(mConstantBuffer_Bounds, 0, nullptr, &cb_bounds, 0, 0);

	DriverLog("[0808][DrawAADT][BeforeVSSetConstantBuffers] ");
	deviceContext->VSSetConstantBuffers(0, 1, &mConstantBuffer_Bounds);
	//deviceContext->VSSetConstantBuffers(0, 1, &mConstantBuffer_Bounds_test);


	DriverLog("[0808][DrawAADT][AfterVSSetConstantBuffers] ");

	deviceContext->PSSetShader(ps_simpleCopy, nullptr, 0);



	




	//SET CB
	VSConstantBuffer picturesize;

	picturesize.x =gConfigReader.GetEveWidth();
	picturesize.y = gConfigReader.GetEveHeight();
	picturesize.cutx = gConfigReader.GetCutx();
	picturesize.cuty = gConfigReader.GetCuty();

	deviceContext->UpdateSubresource(mpConstantBuffer, 0, nullptr, &picturesize, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &mpConstantBuffer);

	PSAdjustmentBuffer adjustment = { 0 };
	adjustment.bright = gConfigReader.GetBrightValue();
	adjustment.saturation = gConfigReader.GetSaturationValue();
	adjustment.contrast = gConfigReader.GetContrastValue();
	adjustment.gamma = gConfigReader.GetGammaValue();
	adjustment.function = gConfigReader.GetSharperValue();


	deviceContext->UpdateSubresource(mpAdjustMentBuffer, 0, nullptr, &adjustment, 0, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &mpAdjustMentBuffer);


	

	//DRAW CALL
	deviceContext->DrawIndexed(m_IndexCount, 0, 0);
	deviceContext->Flush();





}