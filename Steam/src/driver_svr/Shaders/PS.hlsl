
Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

struct PixelShaderInput
{
	float4 pos		: POSITION;
	float2 tex		: TEXCOORD0;
};

float4 PixelShaderF(PixelShaderInput input) : SV_TARGET
{
    float4 texSample = shaderTexture.Sample(SampleType, input.tex);
    float4 result;
    result.a = texSample.a;
    result.rgb = pow(texSample.rgb, 1.0f / 2.2f);

    return result;
}

float4 PixelShaderF1(PixelShaderInput input) : SV_TARGET
{
    return float4(input.tex.x, input.tex.y, 0.0f, 1.0f);
}
