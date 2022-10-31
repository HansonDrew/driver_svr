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
cbuffer VSConstantBuffer : register(b0)
{
	float2 picturesize;
}
Interpolants main(Vertex In)
{
	return In;
}
//Interpolants main(Vertex In)
//{
//	Interpolants output;
//	output.position = float4(In.position.x, In.position.y, 0.0, 1.0);
//	output.texcoord = In.texcoord;
//	return output;
//}
