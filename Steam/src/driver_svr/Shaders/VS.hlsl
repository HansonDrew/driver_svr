// Color converter vertex shader - passthrough

struct VertexShaderInput
{
	float2 pos		: POSITION;
	float2 tex		: TEXCOORD0;
};

struct VertexShaderOutput
{
	float4 pos		: SV_POSITION;
	float2 tex		: TEXCOORD0;
};

VertexShaderOutput VertexShaderF(VertexShaderInput input)
{
	VertexShaderOutput output;
	output.pos = float4(input.pos.x,input.pos.y, 0.5f, 1.0);
	output.tex = input.tex;
	return output;
}