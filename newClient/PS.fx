Texture2D gtxtTexture : register(t0);
SamplerState gSamplerState : register(s0);

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 tex2dcoord : TEXCOORD0;
};

float4 PSTexturedColor(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 cColor = gtxtTexture.Sample(gSamplerState, input.tex2dcoord);

	return(cColor);
}