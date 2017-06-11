cbuffer cbViewProjectionMatrix : register(b0)
{
	matrix gmtxView : packoffset(c0);
	matrix gmtxProjection : packoffset(c4);
};

cbuffer cbWorldMatrix : register(b1)
{
	matrix gmtxWorld : packoffset(c0);
};

struct VS_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 tex2dcoord : TEXCOORD0;
};

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 tex2dcoord : TEXCOORD0;
};

VS_TEXTURED_OUTPUT VSTexturedColor(VS_TEXTURED_INPUT input)
{
    VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
    matrix mtxWorldViewProjection = mul(gmtxWorld, gmtxView);
    mtxWorldViewProjection = mul(mtxWorldViewProjection, gmtxProjection);
    output.position = mul(float4(input.position, 1.0f), mtxWorldViewProjection);
    output.tex2dcoord = input.tex2dcoord;

    return(output);
}