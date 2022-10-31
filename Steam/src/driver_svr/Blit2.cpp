//-----------------------------------------------------------------------------
//  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
//  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
//-----------------------------------------------------------------------------

#include "Blit2.h"
#include "D3DHelper.h"
#include "HLSL/PixelShader.h"
#include "HLSL/VertexShader.h"
#include "HLSL/PixelResizeShader.h"
#include "d3dcompiler.h"
#include "ReadData.h"
#include "config_reader.h"
#include "RVRUtils.h"
#include <d2d1.h>
#include <dwrite.h>
#include "Util.h"
#include "RVRLogger.h"
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
using namespace RVR;
extern ConfigReader gConfigReader;



namespace Blit2 {
	float ulMin, ulMax, vlMin, vlMax, urMin, urMax, vrMin, vrMax;
	float tw_q1[4] = { 1,0,0,0 };
	float tw_q2[4] = { 1,0,0,0 };
    struct QuadVertex {
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

	void SetQ1Q2(float q1[4],float q2[4])
	{
		memmove(tw_q1, q1, sizeof(float) * 4);
		memmove(tw_q2, q2, sizeof(float) * 4);
	}
	struct PSAdjustmentBuffer
	{

		float bright;
		float saturation;
		float contrast;
		float gamma;
		float a;
		float b;
		float c;
		float shaper;
	};


	struct tw_param
	{
		float Rot1ToRot2[4][4];
		float Rot2ToRot1[4][4];

		float ulMin;
		float ulMax;
		float vlMin;
		float vlMax;

		float urMin;
		float urMax;
		float vrMin;
		float vrMax;
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
	ID3D11Buffer *mpConstantBuffer;
	ID3D11Buffer *mpAdjustMentBuffer;
	ID3D11Buffer* tw_buffer_;
	ID3D11Buffer* m_pIndexBuffer;


	ID2D1Factory* m_pd2dFactory;							// D2D工厂
	ID2D1RenderTarget* m_pd2dRenderTarget;				// D2D渲染目标
	IDWriteFactory* m_pdwriteFactory;					// DWrite工厂

	ID2D1SolidColorBrush* m_pColorBrush;	    // 单色笔刷
	IDWriteFont* m_pFont;					// 字体
	IDWriteTextFormat* m_pTextFormat;		// 文本格式

	float mWidth;
	float mHeigth;
	void SetWH(float w, float y) 
	{ mWidth = w; mHeigth = y; };
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
		if (gConfigReader.GetCutFlag() == 1)
		{
			//int64_t begintime=RVR::nowInNs();
			////////// 以三倍 边缘压缩为例 
			//     _________________________
			//    |    |              |    |
			//    |    |              |    |
			//    |    |              |    |
			//    |    |              |    |
			//    |    |              |    |1920
			//    |    |              |    |
			//    |    |              |    |
			//    |    |              |    |
			//    | 480|      960     |480 |
			//    |____|______________|____|
			//               1920
			//
			//     _______________________
			//     |   |              |   |
			//     |   |              |   |
			//     |   |              |   |
			//     |   |              |   |
			//     |   |              |   |1280
			//     |   |              |   |
			//     |   |              |   |
			//     |   |              |   |
			//     |160|      960     |160|
			//     |___|______________|___|
			//               1280
			//480 -> 160   960 部分 原图 映射    顶点映射， 480 是 1920的1/4映射到 160/1280 区域
			float minu = uMin;
			float minv = vMin;
			
			float maxu = uMax;
			float maxv = vMax;
			float originText2dx = gConfigReader.GetEveWidth();
			float originText2dy = gConfigReader.GetEveHeight();
			float cutx = gConfigReader.GetCutx();//  x轴 压缩区域的 width 。480 压缩，960原图 ，480 压缩。依照视觉决定。960的区域不影响视觉感受 （1/4 +1/2+1/4）
			float cuty = gConfigReader.GetCuty();//  y轴 压缩区域的 height
			 

			float xbase = originText2dx / (originText2dx - (cutx - cutx / gConfigReader.GetComPress()) * 2);//x 缩放比例   cutx - cutx / gConfigReader.GetComPress() 计算出 压缩后剩余的 width 。即：480 压成多大
			float ybase = originText2dx / (originText2dy - (cuty - cuty / gConfigReader.GetComPress()) * 2);
			float pictcutx = (maxu - minu) / (originText2dx / cutx);//1/(1920/480) 图片上 1/4的位置。
			float pictcuty = (maxv - minv) / (originText2dy / cuty);
			//float positioncutx = 2.00*((cutx / gConfigReader.GetComPress()) / originText2dx)*xbase; //装载图元时 ，坐标 是  0，0圆心  -1到1 长度的坐标系 长度 2,大图 单边 cut的长度 ，
			//																	 // 240 占 1920 8分之一 ，映射后是  1440 的 1/8*basex , 坐标轴 单位 2 。在新图的位置
			//float posititoncuty = 2.00*((cuty / gConfigReader.GetComPress()) / originText2dy)*ybase;
		    float positioncutx = 2.00f * (cutx / gConfigReader.GetComPress()) / (originText2dx - (cutx - cutx / gConfigReader.GetComPress()) * 2.00f); //装载图元时 ，坐标 是  0，0圆心  -1到1 长度的坐标系 长度 2,大图 单边 cut的长度 ，																 // 240 占 1920 8分之一 ，映射后是  1440 的 1/8*basex , 坐标轴 单位 2 。在新图的位置
	        float posititoncuty = 2.00f * (cuty / gConfigReader.GetComPress()) / (originText2dy - (cuty - cuty / gConfigReader.GetComPress()) * 2.00f);
			QuadVertex v[VERTEXNUM] = {
			{ -1,  1, minu, minv ,0 },//0
			{-1 + positioncutx,1,minu + pictcutx,minv ,0},//1
			{-1 + positioncutx,1 - posititoncuty,minu + pictcutx,minv + pictcuty ,0},//2
			{-1  ,1 - posititoncuty,minu ,minv + pictcuty ,0},//3				
			{  1 - positioncutx,1  , maxu - pictcutx,  minv  ,0 },//4
			{ 1 - positioncutx,  1 - posititoncuty,  maxu - pictcutx, minv + pictcuty  ,0 },//5				
			{ 1  ,  1  ,  maxu  , minv     ,0},//6
			{ 1  ,  1 - posititoncuty,  maxu  , minv + pictcuty  ,0 },//7			
			{ -1  ,  -1 + posititoncuty ,  minu  , maxv - pictcuty   ,0},//8
			{ -1 + positioncutx  ,  -1 + posititoncuty   ,  minu + pictcutx  , maxv - pictcuty  ,0   },//9				
			{ 1 - positioncutx,  -1 + posititoncuty,  maxu - pictcutx,maxv - pictcuty ,0 },//10
			{ 1  ,  -1 + posititoncuty,  maxu  ,maxv - pictcuty  ,0},//11
			{ -1, -1, minu,maxv ,0 },//12
			{ -1 + positioncutx  ,  -1     ,  minu + pictcutx  , maxv    ,0 },//13
			{ 1 - positioncutx,  -1  ,  maxu - pictcutx,maxv   ,0 },//14		 
			{  1, -1,  maxu,  maxv  ,0 },//15		
			};
			WORD indices[] = {
				0,1,3,
				1, 2, 3,
				2,1,4,
				4,5,2,
				5,4,6,
				6,7,5,
				3,2,8,
				2,9,8,
				2,5,9,
				5,10,9,
				5,7,11,
				5,11,10,
				8,9,13,
				8,13,12,
				9,10,13,
				10,14,13,
				10,11,14,
				11,15,14,
			};
			/*int64_t endtime = RVR::nowInNs();
			float kk = (endtime-begintime  ) / 1000.0000;
	        string msg = "indextime=" + std::to_string(kk);

	        RVR_LOG_A(msg.c_str()); */
			deviceContext->UpdateSubresource(mVB, 0, NULL, v, sizeof(QuadVertex) * VERTEXNUM, 0);
			deviceContext->UpdateSubresource(m_pIndexBuffer, 0, NULL, indices, sizeof(WORD) * INDEXNUM, 0);
		}
		else
		 {
			 QuadVertex v[6] = {
			{ -1, -1, uMin, vMax },
			{ -1,  1, uMin, vMin  },
			{  1, -1, uMax, vMax  },
			{  1,  1, uMax, vMin  },
			 };

			 deviceContext->UpdateSubresource(mVB, 0, NULL, v, sizeof(QuadVertex) * 4, 0);
		 }
		
	}
	void UpdateVB2to1(float* leftbounds, float* rightbounds)
	{
		

		if (leftbounds != nullptr)
		{
			ulMin = leftbounds[0];
			ulMax = leftbounds[1];
			vlMin = leftbounds[2];
			vlMax = leftbounds[3];
		}
		else
		{
			ulMin = 0.0f;
			ulMax = 1.0f;
			vlMin = 0.0f;
			vlMax = 1.0f;
		}

		if (rightbounds != nullptr)
		{
			urMin = rightbounds[0];
			urMax = rightbounds[1];
			vrMin = rightbounds[2];
			vrMax = rightbounds[3];
		}
		else
		{
			urMin = 0.0f;
			urMax = 1.0f;
			vrMin = 0.0f;
			vrMax = 1.0f;
		}


		float originText2dx = gConfigReader.GetEveWidth();
		float originText2dy = gConfigReader.GetEveHeight();
		float cutx = gConfigReader.GetCutx();//  x轴 压缩区域的 width 。480 压缩，960原图 ，480 压缩。依照视觉决定。960的区域不影响视觉感受 （1/4 +1/2+1/4）
		float cuty = gConfigReader.GetCuty();//  y轴 压缩区域的 height


		float xbase = originText2dx / (originText2dx - (cutx - cutx / gConfigReader.GetComPress()) * 2);//x 缩放比例   cutx - cutx / gConfigReader.GetComPress() 计算出 压缩后剩余的 width 。即：480 压成多大
		float ybase = originText2dx / (originText2dy - (cuty - cuty / gConfigReader.GetComPress()) * 2);
		float pictcutxl = (ulMax - ulMin) / (originText2dx / cutx);//1/(1920/480) 图片上 1/4的位置。
		float pictcutyl = (vlMax - vlMin) / (originText2dy / cuty);

		float pictcutxr = (urMax - urMin) / (originText2dx / cutx);//1/(1920/480) 图片上 1/4的位置。
		float pictcutyr = (vrMax - vrMin) / (originText2dy / cuty);
		//float positioncutx = 2.00*((cutx / gConfigReader.GetComPress()) / originText2dx)*xbase; //装载图元时 ，坐标 是  0，0圆心  -1到1 长度的坐标系 长度 2,大图 单边 cut的长度 ，
		//																	 // 240 占 1920 8分之一 ，映射后是  1440 的 1/8*basex , 坐标轴 单位 2 。在新图的位置
		//float posititoncuty = 2.00*((cuty / gConfigReader.GetComPress()) / originText2dy)*ybase;
		float positioncutx = 2.00f * (cutx / gConfigReader.GetComPress()) / (originText2dx - (cutx - cutx / gConfigReader.GetComPress()) * 2.00f) / 2; //装载图元时 ，坐标 是  0，0圆心  -1到1 长度的坐标系 长度 2,大图 单边 cut的长度 ，																 // 240 占 1920 8分之一 ，映射后是  1440 的 1/8*basex , 坐标轴 单位 2 。在新图的位置
		float posititoncuty = 2.00f * (cuty / gConfigReader.GetComPress()) / (originText2dy - (cuty - cuty / gConfigReader.GetComPress()) * 2.00f);
		QuadVertex v[36] = {

		{ -1,  1, ulMin, vlMin ,0 },//0
		{ -1 + positioncutx,1,ulMin + pictcutxl,vlMin,0 },//1
		{ -1 + positioncutx,1 - posititoncuty,ulMin + pictcutxl,vlMin + pictcutyl,0 },//2
		{ -1  ,1 - posititoncuty,ulMin ,vlMin + pictcutyl,0 },//3
		//////

		{ 0 - positioncutx,1  , ulMax - pictcutxl,  vlMin ,0 },//4
		{ 0 - positioncutx,  1 - posititoncuty,  ulMax - pictcutxl, vlMin + pictcutyl ,0 },//5
		/////
		{ 0  ,  1  ,  ulMax  , vlMin   ,0 },//6
		{ 0  ,  1 - posititoncuty,  ulMax  , vlMin + pictcutyl ,0 },//7
		//////////////////////////////////////////////////////////////////////////
		{ -1  ,  -1 + posititoncuty ,  ulMin  , vlMax - pictcutyl  ,0 },//8
		{ -1 + positioncutx  ,  -1 + posititoncuty   ,  ulMin + pictcutxl  , vlMax - pictcutyl   ,0 },//9
		//////////////////////////
		{ 0 - positioncutx,  -1 + posititoncuty,  ulMax - pictcutxl,vlMax - pictcutyl,0 },//10
		{ 0  ,  -1 + posititoncuty,  ulMax  ,vlMax - pictcutyl ,0 },//11
		{ -1, -1, ulMin,vlMax ,0 },//12
		{ -1 + positioncutx  ,  -1     ,  ulMin + pictcutxl  , vlMax    ,0 },//13
		{ 0 - positioncutx,  -1  ,  ulMax - pictcutxl,vlMax   ,0 },//14		 
		{ 0, -1,  ulMax,  vlMax  ,0 },//15		
		/////
		{0 + positioncutx,1,urMin + pictcutxr,vrMin ,1},//16
		{0 + positioncutx,1 - posititoncuty,urMin + pictcutxr,vrMin + pictcutyr ,1},//17

		{ 1 - positioncutx,1  , urMax - pictcutxr,  vrMin ,1},//18
		{1 - positioncutx,  1 - posititoncuty,  urMax - pictcutxr, vrMin + pictcutyr ,1 },//19

		{ 1  ,  1  ,  urMax  , vrMin   ,1 },//20
		{ 1  ,  1 - posititoncuty,  urMax  , vrMin + pictcutyr ,1 },//21

		{ 0 + positioncutx  ,  -1 + posititoncuty   ,  urMin + pictcutxr  , vrMax - pictcutyr   ,1 },//22
		{ 1 - positioncutx,  -1 + posititoncuty,  urMax - pictcutxr,vrMax - pictcutyr,1 },//23
		{ 1  ,  -1 + posititoncuty,  urMax  ,vrMax - pictcutyr ,1},//24

		{ 0 + positioncutx  ,  -1     ,  urMin + pictcutxr  , vrMax    ,1},//25
		{1 - positioncutx,  -1  ,  urMax - pictcutxr,vrMax   ,1},//26	 
		{  1, -1,  urMax,  vrMax  ,1},//27
		{ 0  ,  1  ,  urMin  , vrMin   ,1 },//6 VIEW1 28 
		{ 0  ,  1 - posititoncuty,  urMin  , vrMin + pictcutyr ,1 },//7  VIEW1  29
		{ 0  ,  -1 + posititoncuty,  urMin  ,vrMax - pictcutyr ,1 },//11  VIEW1  30

		{  0, -1,  urMin,  vrMax  ,1},//15  VIEW1  31		

		};
		WORD indices[] = {
		0,1,3,
		1, 2, 3,
		2,1,4,
		4,5,2,
		5,4,6,
		6,7,5,
		3,2,8,
		2,9,8,
		2,5,9,
		5,10,9,
		5,7,11,
		5,11,10,
		8,9,13,
		8,13,12,
		9,10,13,
		10,14,13,
		10,11,14,
		11,15,14,
		///
		28,16,29,
		16,17,29,
		16,18,17,
		18,19,17,
		18,20,19,
		20,21,19,
		29,17,30,
		17,22,30,
		17,19,22,
		19,23,22,
		19,21,23,
		21,24,23,
		30,22,31,
		22,25,31,
		22,23,25,
		23,26,25,
		23,24,26,
		24,27,26
		};
 
		deviceContext->UpdateSubresource(mVB, 0, NULL, v, sizeof(QuadVertex) * VERTEXNUM2to1, 0);
		deviceContext->UpdateSubresource(m_pIndexBuffer, 0, NULL, indices, sizeof(WORD) * INDEXNUM2to1, 0);



	}
	void Blit2(ID3D11RenderTargetView* dst, D3D11_VIEWPORT* dstViewport, ID3D11ShaderResourceView* src,  ID3D11DepthStencilView*depsrc, float* srcBounds, bool blendingEnabled)
	{
		 
		UINT stride = sizeof(QuadVertex);
		UINT offset = 0;
        deviceContext->ClearState();
		{ //src
			ID3D11ShaderResourceView* srcin[2];
			srcin[0] = src;
			srcin[1] = src;

			deviceContext->PSSetSamplers(0, 1, &ss);
			deviceContext->PSSetShaderResources(0, 1, &src);
		}
		{ //dst
			deviceContext->OMSetRenderTargets(1, &dst, nullptr);
			deviceContext->RSSetViewports(1, dstViewport);
		}

		{ //quad
			UpdateVB(srcBounds);
			deviceContext->IASetInputLayout(il);
			deviceContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
			if (gConfigReader.GetCutFlag()==1)
			{
				deviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
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

        deviceContext->RSSetState(rs);
		deviceContext->VSSetShader(_vs, nullptr, 0);
		deviceContext->PSSetShader(ps_simpleCopy, nullptr, 0);
		VSConstantBuffer picturesize;

		picturesize.x = dstViewport->Width;
		picturesize.y = dstViewport->Height;
		picturesize.cutx = gConfigReader.GetCutx();
		picturesize.cuty = gConfigReader.GetCuty();

		deviceContext->UpdateSubresource(mpConstantBuffer, 0, nullptr, &picturesize, 0, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &mpConstantBuffer);

		PSAdjustmentBuffer adjustment;
		adjustment.bright = gConfigReader.GetBrightValue();
		adjustment.saturation = gConfigReader.GetSaturationValue();
		adjustment.contrast = gConfigReader.GetContrastValue();
		if ((0x8000 & GetAsyncKeyState('S')) != 0 && (0x8000 & GetAsyncKeyState('O')) != 0)
		{
			adjustment.shaper = 1;
		}
		else 
		{
			adjustment.shaper = 1.6;
		}
		
		deviceContext->UpdateSubresource(mpAdjustMentBuffer, 0, nullptr, &adjustment, 0, 0);
		deviceContext->PSSetConstantBuffers(1, 1, &mpAdjustMentBuffer);
		if (gConfigReader.GetCutFlag()==1)
		{
			deviceContext->DrawIndexed(INDEXNUM, 0, 0);
		} 
		else
		{
			deviceContext->Draw(4, 0);
		}
		
		deviceContext->Flush();
	}
	void Blit2to1(ID3D11RenderTargetView* dst, D3D11_VIEWPORT* dstViewport, ID3D11ShaderResourceView* src[2], float* leftbounds, float* rightbounds, bool blendingEnabled)
	{
		UINT stride = sizeof(QuadVertex);
		UINT offset = 0;
		deviceContext->ClearState();
		{ //src
			deviceContext->PSSetSamplers(0, 1, &ss);
			deviceContext->PSSetShaderResources(0, 2, src);
		}
		{ //dst
			deviceContext->OMSetRenderTargets(1, &dst, nullptr);
			deviceContext->RSSetViewports(1, dstViewport);
		}

		{ //quad
			UpdateVB2to1(leftbounds, rightbounds);
			deviceContext->IASetInputLayout(il);
			deviceContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
			if (gConfigReader.GetCutFlag() == 1)
			{
				deviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
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
		else
		{
			deviceContext->OMSetBlendState(blendStateF, 0, 0xffffffff);
		}
		deviceContext->RSSetState(rs);
		deviceContext->VSSetShader(_vs, nullptr, 0);

		deviceContext->PSSetShader(ps_simpleCopy, nullptr, 0);
		VSConstantBuffer picturesize;

		picturesize.x = gConfigReader.GetEveWidth();
		picturesize.y = gConfigReader.GetEveHeight();
		picturesize.cutx = gConfigReader.GetCutx();
		picturesize.cuty = gConfigReader.GetCuty();

		deviceContext->UpdateSubresource(mpConstantBuffer, 0, nullptr, &picturesize, 0, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &mpConstantBuffer);
		
		PSAdjustmentBuffer adjustment = {0};
		adjustment.bright = gConfigReader.GetBrightValue();
		adjustment.saturation = gConfigReader.GetSaturationValue();
		adjustment.contrast = gConfigReader.GetContrastValue();
		adjustment.gamma = gConfigReader.GetGammaValue();
		if ((0x8000 & GetAsyncKeyState('S')) != 0 && (0x8000 & GetAsyncKeyState('O')) != 0)
		{
			adjustment.shaper =3.5;
		}
		else
		{
			adjustment.shaper = 1;
		}
		adjustment.shaper = gConfigReader.GetSharperValue();
	 
		
		deviceContext->UpdateSubresource(mpAdjustMentBuffer, 0, nullptr, &adjustment, 0, 0);
		deviceContext->PSSetConstantBuffers(1, 1, &mpAdjustMentBuffer);

		tw_param tw = { 0 };
		
		float q1to2 [4]= { 0 };
		float q2to1[4] = { 0 };
	
		GetRotationFromQuat1ToQuat2(tw_q1,tw_q2, q1to2,q2to1);
		float mat_[16] = { 0 };

		ConvertQuatToMatrix(q1to2, mat_);
		memmove(tw.Rot1ToRot2, mat_, sizeof(mat_));
		ConvertQuatToMatrix(q2to1, mat_);
		memmove(tw.Rot2ToRot1, mat_, sizeof(mat_));
		double pitch = 0;
		double yaw = 0;
		double roll = 0;
		RotationToEularAngles2(q2to1,pitch,yaw,roll);
		//RVR::RVR_LOG_A("time warp  pitch=%lf yaw=%lf roll=%lf", pitch,yaw,roll);
		tw.ulMin = ulMin;
		tw.vlMin = vlMin;
		tw.vrMin = vrMin;
		tw.urMin = urMin;
		tw.ulMax = ulMax;
		tw.vlMax = vlMax;
		tw.vrMax = vrMax;
		tw.urMax = urMax;
		deviceContext->UpdateSubresource(tw_buffer_, 0, nullptr, &tw, 0, 0);
		deviceContext->PSSetConstantBuffers(2, 1, &tw_buffer_);


		deviceContext->DrawIndexed(INDEXNUM2to1, 0, 0);
		deviceContext->Flush();
	}
	void Resize(ID3D11RenderTargetView * dst, D3D11_VIEWPORT * dstViewport, ID3D11ShaderResourceView * src, ID3D11DepthStencilView * depsrc)
	{

		QuadVertex v[6] = {
			{ -1, -1, 0, 1 },
			{ -1,  1, 0, 0  },
			{  1, -1, 1, 1  },
			{  1,  1, 1, 0  },
		};

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

		 
		{
			deviceContext->OMSetBlendState(blendStateT, 0, 0xffffffff);
		}

		 
		deviceContext->VSSetShader(_vs, nullptr, 0);
		deviceContext->PSSetShader(ps_simpleResize, nullptr, 0);
		/*VSConstantBuffer picturesize;
		 
		picturesize.x = 1920;
		picturesize.y = 1920;
		picturesize.cutx = gConfigReader.GetCutx();
		picturesize.cuty = gConfigReader.GetCuty();
		 
		deviceContext->UpdateSubresource(mpConstantBuffer, 0, nullptr, &picturesize, 0, 0);
		deviceContext->PSSetConstantBuffers(0, 1,&mpConstantBuffer);*/
		deviceContext->Draw(4, 0);
		deviceContext->Flush();

	}

	//void DepthResize(ID3D11RenderTargetView* dst, D3D11_VIEWPORT* dstViewport, ID3D11ShaderResourceView* src, ID3D11DepthStencilView* depsrc)
	//{

	//	QuadVertex v[6] = {
	//		{ -1, -1, 0, 1 },
	//		{ -1,  1, 0, 0  },
	//		{  1, -1, 1, 1  },
	//		{  1,  1, 1, 0  },
	//	};

	//	UINT stride = sizeof(QuadVertex);
	//	UINT offset = 0;
	//	deviceContext->ClearState();
	//	{ //src
	//		deviceContext->PSSetSamplers(0, 1, &ss);
	//		deviceContext->PSSetShaderResources(0, 1, &src);
	//	}
	//	{ //dst
	//		deviceContext->OMSetRenderTargets(1, &dst, nullptr);
	//		deviceContext->RSSetViewports(1, dstViewport);
	//	}

	//	{ //quad

	//		deviceContext->IASetInputLayout(il);
	//		deviceContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	//		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//	}


	//	{
	//		deviceContext->OMSetBlendState(blendStateT, 0, 0xffffffff);
	//	}


	//	deviceContext->VSSetShader(_vs, nullptr, 0);
	//	deviceContext->PSSetShader(ps_simpleCopy, nullptr, 0);
	//	 
	//	deviceContext->Draw(4, 0);
	//	deviceContext->Flush();

	//}
	void CreateBlit2Resources(ID3D11Device* device)
	{
		 
		device = device;
		device->GetImmediateContext(&deviceContext);
		{
			D3D11_BUFFER_DESC cbd;
			ZeroMemory(&cbd, sizeof(cbd));
			cbd.Usage = D3D11_USAGE_DEFAULT; //D3D11_USAGE_DYNAMIC;
			cbd.ByteWidth = sizeof(VSConstantBuffer);
			cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbd.CPUAccessFlags = 0;// D3D11_CPU_ACCESS_WRITE;
			cbd.MiscFlags = 0;
			cbd.StructureByteStride = 0;
			HR(device->CreateBuffer(&cbd, NULL,&mpConstantBuffer));

			cbd.ByteWidth = sizeof(PSAdjustmentBuffer);
			HR(device->CreateBuffer(&cbd, NULL, &mpAdjustMentBuffer));

			cbd.ByteWidth = sizeof(tw_param);
			HR(device->CreateBuffer(&cbd, NULL, &tw_buffer_));


		}
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
			blendDesc.AlphaToCoverageEnable = false;
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
            blendDesc.AlphaToCoverageEnable = false;
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
			device->CreatePixelShader(g_PixelShader, sizeof(g_PixelShader), nullptr, &ps_simpleCopy);
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
			if (gConfigReader.GetCutFlag() == 1)
			{
				if (gConfigReader.BigPicture())
				{
					bufferDesc.ByteWidth = sizeof(QuadVertex) * VERTEXNUM2to1;
				}
				else
				{
					bufferDesc.ByteWidth = sizeof(QuadVertex) * VERTEXNUM;
				}

			}
			else
			{
				bufferDesc.ByteWidth = sizeof(QuadVertex) * 6;
			}

			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			device->CreateBuffer(&bufferDesc, NULL, &mVB);
			if (gConfigReader.GetCutFlag() == 1)
			{
				D3D11_SUBRESOURCE_DATA InitData;
				::memset(&InitData, 0, sizeof(InitData));

				DWORD indices[INDEXNUM];
				// 设置索引缓冲区描述
				D3D11_BUFFER_DESC ibd;
				ZeroMemory(&ibd, sizeof(ibd));
				ibd.Usage = D3D11_USAGE_DEFAULT;
				ibd.ByteWidth = sizeof(DWORD) * INDEXNUM;
				ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
				ibd.CPUAccessFlags = 0;
				// 新建索引缓冲区
				InitData.pSysMem = indices;
				device->CreateBuffer(&ibd, &InitData, &m_pIndexBuffer);
			}
			
		}
		
	}
	void CreateTextResources(ID3D11Texture2D* d3d11Texture) 
	{
		HRESULT hr;
		hr=D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pd2dFactory);
		hr=DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_pdwriteFactory));
		IDXGISurface* surface;
		hr = d3d11Texture->QueryInterface(__uuidof(IDXGISurface), reinterpret_cast<void**>(&surface));
		D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));
		hr = m_pd2dFactory->CreateDxgiSurfaceRenderTarget(surface, &props, &m_pd2dRenderTarget );
		surface->Release();
		if (hr == E_NOINTERFACE)
		{
			OutputDebugStringW(L"\n警告：Direct2D与Direct3D互操作性功能受限，你将无法看到文本信息。现提供下述可选方法：\n"
				L"1. 对于Win7系统，需要更新至Win7 SP1，并安装KB2670838补丁以支持Direct2D显示。\n"
				L"2. 自行完成Direct3D 10.1与Direct2D的交互。详情参阅："
				L"https://docs.microsoft.com/zh-cn/windows/desktop/Direct2D/direct2d-and-direct3d-interoperation-overview""\n"
				L"3. 使用别的字体库，比如FreeType。\n\n");
		}
		else if (hr == S_OK)
		{
			// 创建固定颜色刷和文本格式
			m_pd2dRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&m_pColorBrush);
			m_pdwriteFactory->CreateTextFormat(L"宋体", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 70, L"zh-cn",&m_pTextFormat);
		}
		else
		{
			// 报告异常问题
			OutputDebugStringW(L"d2d text error");
		}
	}
	void WirteText(WCHAR* text)
	{
		if (m_pd2dRenderTarget != nullptr)
		{
			m_pd2dRenderTarget->BeginDraw();			

			m_pd2dRenderTarget->DrawTextW(text, (UINT32)wcslen(text), m_pTextFormat ,
				D2D1_RECT_F{ 400.0f, 400.0f,1200.0f, 1200.0f }, m_pColorBrush );
			HR(m_pd2dRenderTarget->EndDraw());
		}
		deviceContext->Flush();
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