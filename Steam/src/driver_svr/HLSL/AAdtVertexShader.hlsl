struct Vertex
{
	float4 position     : POSITION;
	float2 texcoord     : TEXCOORD0;
	uint    View : VIEW;
};

struct Interpolants
{
	float4 position     : SV_Position;
	float2 texcoord     : TEXCOORD0;
	uint    View : VIEW;
};

cbuffer VSConstantBuffer_Bounds : register(b0)
{
	float ulMin;
	float ulMax;
	float vlMin;
	float vlMax;

	float urMin;
	float urMax;
	float vrMin;
	float vrMax;
}


Interpolants main(Vertex In)
{
	Interpolants Out;
	Out.View = In.View;
	if (In.View == 0)
	{
		Out.position.x = In.position.x * 0.5f - 0.5f;
		Out.position.y = In.position.y;
		Out.position.z = In.position.z;
		Out.position.w = In.position.w;

		Out.texcoord.x = In.texcoord.x * (ulMax - ulMin) + ulMin;
		Out.texcoord.y = In.texcoord.y * (vlMax - vlMin) + vlMin;

	}
	else
	{
		Out.position.x = In.position.x * 0.5f + 0.5f;
		Out.position.y = In.position.y;
		Out.position.z = In.position.z;
		Out.position.w = In.position.w;

		Out.texcoord.x = In.texcoord.x * (urMax - urMin) + urMin;
		Out.texcoord.y = In.texcoord.y * (vrMax - vrMin) + vrMin;

	}

	return Out;
}

