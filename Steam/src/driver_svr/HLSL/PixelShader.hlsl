cbuffer VSConstantBuffer : register(b0)
{
	 
	float2 picturesize;
	float2 cutxy;

}

cbuffer VSConstantBuffer : register(b1)
{

	float bright;
	float saturation;
	float contrast;
	float gamma;
	float a;
	float b;
	float c;
	float sharper;

}

// ------------------------------------------------ 
// STEP 3 
// PS传入两个矩阵
// PS传入bound参数
//-------------------------------------------------


cbuffer CB_Rotation : register(b2)
{
	matrix Rot1ToRot2;
	matrix Rot2ToRot1;

	float ulMin;
	float ulMax;
	float vlMin;
	float vlMax;

	float urMin;
	float urMax;
	float vrMin;
	float vrMax;

}


struct Interpolants
{
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD0;
	uint    View : VIEW;
};
Texture2D LtxDiffuse : register(t0);
Texture2D RtxDiffuse : register(t1);
SamplerState bilinearSampler {
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};
SamplerState samLinear : register(s0);
SamplerState MeshTextureSampler
{
	Filter = D3D11_FILTER_ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
};
struct Pixel
{
	float4 color;
};
const static float DX = 1.f/ picturesize.x;
const static float DY = 1.f/ picturesize.y;
const static float sharpenNeighbourWeight = -sharper / 8.f;

float4 sharpen(Interpolants In)
{
	float2 u_TextureCoordOffset_Dis1[9] = {
		
		float2(-1., 1.), float2(0. , 1.), float2(1., 1.) ,
	    float2(-1., 0.), float2(0. , 0.), float2(1. , 0.) ,
		float2(-1.,-1.), float2(0. , -1.), float2(1. , -1.) 
		
	};
	float4 Outcolor;
	float2 texcoord = float2(In.texcoord.x, In.texcoord.y);
	float4 sample[9];
	for (int i = 0; i < 9; i++)
	{
		if (In.View == (uint)0) { // Left View
			sample[i] = LtxDiffuse.Sample(samLinear, texcoord + u_TextureCoordOffset_Dis1[i] / picturesize.x);
		}
		else if (In.View == (uint)1)
		{
			sample[i] = RtxDiffuse.Sample(samLinear, texcoord + u_TextureCoordOffset_Dis1[i] / picturesize.x);
		}
		
	}
	Outcolor = (
		(0.0  * (sample[0] + sample[2] + sample[6] + sample[8])) +
		(-1 * (sample[1] + sample[3] + sample[5] + sample[7])) +
		(5.0 * sample[4])
		)  ;
	return Outcolor;
}

float4 GetSharpenNeighborComponent(float2 uv, float xoff, float yoff,uint view) {
	float4 pixel = float4(0,0,0,0);
	if (view == (uint)0) { // Left View
		 
		pixel= LtxDiffuse.Sample(bilinearSampler, uv + float2(xoff, yoff)).rgba * sharpenNeighbourWeight;
	}
	else if (view == (uint)1)
	{
		pixel = RtxDiffuse.Sample(bilinearSampler, uv + float2(xoff, yoff)).rgba * sharpenNeighbourWeight;
	}
	return pixel;
}
float4 sharpen2(Interpolants In)
{
	float4 pixel;
	float2 uv = float2(In.texcoord.x, In.texcoord.y);

	if (In.View == (uint)0) { // Left View
		pixel = LtxDiffuse.Sample(bilinearSampler, uv).rgba * (sharper + 1.f);
	}
	else if (In.View == (uint)1)
	{
		pixel = RtxDiffuse.Sample(bilinearSampler, uv).rgba * (sharper+ 1.f);
	}
	
	
	pixel += GetSharpenNeighborComponent(uv, -DX, -DY, In.View);
	pixel += GetSharpenNeighborComponent(uv, 0, -DY, In.View);
	pixel += GetSharpenNeighborComponent(uv, +DX, -DY, In.View);
	pixel += GetSharpenNeighborComponent(uv, +DX, 0, In.View);
	pixel += GetSharpenNeighborComponent(uv, +DX, +DY, In.View);
	pixel += GetSharpenNeighborComponent(uv, 0, +DY, In.View);
	pixel += GetSharpenNeighborComponent(uv, -DX, +DY, In.View);
	pixel += GetSharpenNeighborComponent(uv, -DX, 0, In.View);
	return pixel;
}
//
//
//float4 doGaussBlurDis2(float2 texcoord)
//{
//	float2 u_TextureCoordOffset_Dis2[25] = {
//		float2(-4, 4), float2(-2, 4), float2(0, 4), float2(2, 4), float2(4, 4),
//		float2(-4, 2), float2(-2, 2), float2(0, 2), float2(2, 2), float2(4, 2),
//		float2(-4, 0), float2(-2, 0), float2(0, 0), float2(2, 0), float2(4, 0),
//		float2(-4, -2), float2(-2, -2), float2(0, -2), float2(2, -2), float2(4, -2),
//		float2(-4, -4), float2(-2, -4), float2(0, -4), float2(2, -4), float2(4, -4)
//	};
//	float4 Outcolor;
//	float2 uv = float2(picturesize.x, picturesize.y);
//	float4 sample[25];
//	for (int i = 0; i < 25; i++)
//	{
//		sample[i] = txDiffuse.Sample(samLinear, texcoord + u_TextureCoordOffset_Dis2[i] / picturesize.x);
//	}
//	Outcolor = (
//		(1.0  * (sample[0] + sample[4] + sample[20] + sample[24])) +
//		(4.0  * (sample[1] + sample[3] + sample[5] + sample[9] + sample[15] + sample[19] + sample[21] + sample[23])) +
//		(7.0  * (sample[2] + sample[10] + sample[14] + sample[22])) +
//		(16.0 * (sample[6] + sample[8] + sample[16] + sample[18])) +
//		(26.0 * (sample[7] + sample[11] + sample[13] + sample[17])) +
//		(41.0 * sample[12])
//		) / 273.0;
//	return Outcolor;
//}
//
 
float3 brightnessfun(float3 rgb)
{
	float3 finalColor = rgb * bright;
	return finalColor;
}

float3 saturationfun(float3 rgb)
{
	float luminance = 0.2125 * rgb.r + 0.7154 * rgb.g + 0.0721 * rgb.b;
	float3 luminanceColor = float3(luminance, luminance, luminance);
	//使用_Saturation和其上一步得到的颜色之间进行插值，得到希望的饱和度
	float3 finalColor = lerp(luminanceColor, rgb, saturation);
	return finalColor;
}

float3 contrastfun(float3 rgb)
{
	float3 avgColor = float3(0.5, 0.5, 0.5);
	//使用_Contrast在其和上一步得到的颜色之间进行插值
	float3 finalColor = lerp(avgColor, rgb, contrast);

	return finalColor;
}

float3 gammafun(float3 rgb)
{
	float3 finalColor = pow(rgb , gamma)  ;
	 

	return finalColor;
}


float4 adjustment(float4 rgba)
{
	float4 retval_rgba = rgba;
	 
	if (abs(bright-1)>0.0001)
	{
		retval_rgba.rgb = brightnessfun(retval_rgba.rgb);
	}
    if (abs(saturation - 1) > 0.0001)
	{
		retval_rgba.rgb = saturationfun(retval_rgba.rgb);
	}
	if (abs(contrast - 1) > 0.0001)
	{
		retval_rgba.rgb = contrastfun(retval_rgba.rgb);
	}
	if (abs(gamma - 1) > 0.0001)
	{
		retval_rgba.rgb = gammafun(retval_rgba.rgb);
	}
	return retval_rgba;
}



	// ------------------------------------------------ 
	// STEP 4 
	// 修改PS
	//-------------------------------------------------

	Pixel main(Interpolants In) : SV_Target
	{
		//4-1 从 将纹理 坐标扩展到 0-1，去除bounds的影响
		//float2 tex_uv;
		//if (In.View == (uint)0)
		//{
		//	tex_uv.x = (In.texcoord.x - ulMin) / (ulMax - ulMin);
		//	tex_uv.y = (In.texcoord.y - vlMin) / (vlMax - vlMin);
		//}

		//if (In.View == (uint)1)
		//{
		//	tex_uv.x = (In.texcoord.x - urMin) / (urMax - urMin);
		//	tex_uv.y = (In.texcoord.y - vrMin) / (vrMax - vrMin);
		//}

		////4-2 将0-1的UV扩展到-1到1的NDC
		//float4 NDC_coord;
		//NDC_coord.x = tex_uv.x * 2.0f - 1.0f;
		//NDC_coord.y = tex_uv.y * -2.0f + 1.0f;
		//NDC_coord.z = 1.0f;
		//NDC_coord.w = 1.0f;


		////4-3 旋转 Rot2ToRot1  Rot1ToRot2
		//float4 NDC_new = mul(NDC_coord, Rot2ToRot1);

		////4-4 归一到同一个深度
		//float2 flattend = NDC_new.xy / NDC_new.z;


		////4-5 回到纹理做标
		//float2 tex_new;
		//tex_new.x = flattend.x * 0.5f + 0.5f;
		//tex_new.y = flattend.y * -0.5f - 0.5f;


		////4-6用新的坐标采样
		//Pixel Out;
		//if (In.View == (uint)0) {
		//Out.color = LtxDiffuse.Sample(samLinear, tex_new);
		//}
		//else if (In.View == (uint)1)
		//{
		//Out.color = RtxDiffuse.Sample(samLinear, tex_new);
		//}

		//Out.color = adjustment(Out.color);
		//if (function == 1)
		//{
		//	Out.color = sharpen(In);
		//}


		//return Out;
	Pixel Out;
	if (abs(sharper - 1) > 0.0001)
	{
		    Out.color = sharpen2(In);
	}
	else
	{
	        if (In.View == (uint)0) 
			{ // Left View
			Out.color = LtxDiffuse.Sample(samLinear, In.texcoord);
	        }
	        else if (In.View == (uint)1)
		    {
			Out.color = RtxDiffuse.Sample(samLinear, In.texcoord);
		    }
	}
	Out.color = adjustment(Out.color);
	return Out;
}