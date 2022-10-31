cbuffer VSConstantBuffer : register(b0)
{
	 
	float2 picturesize;
	float2 cutxy;

}
struct Interpolants
{
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD0;

};
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

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);
Pixel resizelittle(Interpolants In)
{
	Pixel Out;
	float cutx = cutxy.x;
	float cuty = cutxy.y;
	float halfcutx = cutxy.x / 2.000000;
	float halfcuty = cutxy.y / 2.000000;

	float littlepw = picturesize.x - cutx;
	float littleph = picturesize.y - cuty;
	
	if ((In.texcoord.x > halfcutx / littlepw ) && (In.texcoord.x < (littlepw - halfcutx) / littlepw)
		&& (In.texcoord.y > halfcuty / littleph) && (In.texcoord.y < (littleph - halfcuty) / littleph))
	{
		In.texcoord.x = In.texcoord.x * littlepw / picturesize.x + halfcutx / picturesize.x;
		In.texcoord.y = In.texcoord.y * littleph / picturesize.y + halfcuty / picturesize.y;
		Out.color = txDiffuse.Sample(samLinear, In.texcoord);
		
	}
	else
	{
		if ((In.texcoord.x <= halfcutx / littlepw) && (In.texcoord.y <= halfcuty / littleph))//lefttop
		{
			In.texcoord.x = In.texcoord.x * littlepw / picturesize.x * 2;
			In.texcoord.y = In.texcoord.y * littleph / picturesize.y * 2;
			Out.color = txDiffuse.Sample(MeshTextureSampler, In.texcoord);

		}

		else if ((In.texcoord.x <= halfcutx / littlepw) && (In.texcoord.y >= (littleph - halfcuty) / littleph))//lefbottom
		{
			In.texcoord.x = In.texcoord.x * littlepw / picturesize.x * 2;
			float originy = In.texcoord.y* littleph / picturesize.y + halfcuty / picturesize.y;
			float basey = littleph / picturesize.y;
			In.texcoord.y = (originy + (originy - basey));
			Out.color = txDiffuse.Sample(MeshTextureSampler, In.texcoord);
		}
		else if ((In.texcoord.x >= (littlepw - halfcutx) / littlepw)
			&& (In.texcoord.y <= halfcuty / littleph))//righttop
		{
			float originx = In.texcoord.x * littlepw / picturesize.x + halfcutx / picturesize.x;
			float basex = littlepw / picturesize.x;
			In.texcoord.x = (originx + (originx - basex));
			In.texcoord.y = In.texcoord.y * littleph / picturesize.y * 2;
			Out.color = txDiffuse.Sample(MeshTextureSampler, In.texcoord);
		}

		else if (In.texcoord.x >= (littlepw - halfcutx) / littlepw
			&& In.texcoord.y >= (littleph - halfcuty) / littleph)//righbottom
		{
			float originx = In.texcoord.x * littlepw / picturesize.x + halfcutx / picturesize.x;
			float basex = littlepw / picturesize.x;
			In.texcoord.x = (originx + (originx - basex));
			float originy = In.texcoord.y* littleph / picturesize.y + halfcuty / picturesize.y;
			float basey = littleph / picturesize.y;
			In.texcoord.y = (originy + (originy - basey));
			Out.color = txDiffuse.Sample(MeshTextureSampler, In.texcoord);
		}

		else if ((In.texcoord.x > 0 && In.texcoord.x <halfcutx / littlepw || In.texcoord.x >(littlepw - halfcutx) / littlepw) &&
			(In.texcoord.y > halfcuty / littleph) &&
			(In.texcoord.y < (littleph - halfcuty) / littleph))//left   right cut x, 
		{
			if (In.texcoord.x < halfcutx / littlepw)
			{
				In.texcoord.x = In.texcoord.x * littlepw / picturesize.x * 2;
			}
			else
			{
				float originx = In.texcoord.x * littlepw / picturesize.x + halfcutx / picturesize.x;
				float basex = littlepw / picturesize.x;
				In.texcoord.x = (originx + (originx - basex));
			}
			In.texcoord.y = In.texcoord.y * littleph / picturesize.y + halfcuty / picturesize.y;
			Out.color = txDiffuse.Sample(MeshTextureSampler, In.texcoord);

		}

		else if ((In.texcoord.y < halfcuty / littleph || In.texcoord.y >(littleph - halfcuty) / littleph) &&
			In.texcoord.x > halfcutx / littlepw &&
			In.texcoord.x < (littlepw - halfcutx) / littlepw)//top  bottom cut y
		{
			if (In.texcoord.y < halfcuty / littleph)
			{
				In.texcoord.y = In.texcoord.y * littleph / picturesize.y * 2;
			}
			else
			{
				float originy = In.texcoord.y* littleph / picturesize.y + halfcuty / picturesize.y;
				float basey = littleph / picturesize.y;
				In.texcoord.y = (originy + (originy - basey));
			}
			In.texcoord.x = In.texcoord.x * littlepw / picturesize.x + halfcutx / picturesize.x;
			Out.color = txDiffuse.Sample(MeshTextureSampler, In.texcoord);
		}

	}
	return Out;
}
Pixel  main(Interpolants In) : SV_Target
{
	 
    Pixel Out;
    Out.color = txDiffuse.Sample(samLinear, In.texcoord);
	Out.color.r = (Out.color.r * 30 + Out.color.g * 59 + Out.color.b * 11 + 50) / 100;
	 
	return Out;
}

//unsigned int  main(Interpolants In) : SV_Target
//{
//	unsigned int out_depth;
//	Pixel Out;
//	Out.color = txDiffuse.Sample(samLinear, In.texcoord);
//	out_depth = (Out.color.r * 30 + Out.color.g * 59 + Out.color.b * 11 + 50) / 100;
//	return out_depth;
//}