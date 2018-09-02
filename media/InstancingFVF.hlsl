cbuffer cbPerObject : register(c0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}

struct VS_INPUT
{
	float4 vPosition: POSITION;
	float3 vNormal  : NORMAL;
	float4 vColorD  : COLOR;
	float2 tCoord   : TEXCOORD0;
	float4x4 iMatrix: TEXCOORD1;
};

struct VS_OUTPUT
{
	float4 vPosition  : POSITION;
	float2 tCoord     : TEXCOORD1;
	float4 vColorD	  : COLOR0;
};

VS_OUTPUT vsmain(VS_INPUT input)
{
	VS_OUTPUT output;
	input.vPosition.w = 1.0f;

	output.vPosition = mul(input.vPosition, transpose(input.iMatrix));
	output.vPosition = mul(output.vPosition, worldMatrix);
	output.vPosition = mul(output.vPosition, viewMatrix);
	output.vPosition = mul(output.vPosition, projectionMatrix);

	output.vColorD = input.vColorD.bgra;
	output.tCoord = input.tCoord;

	return output;
}

Texture2D tex1 : register(t0);

SamplerState sampler1 : register(s1);

float4 psmain(VS_OUTPUT input) : SV_Target
{
	return tex1.Sample(sampler1, input.tCoord).bgra * input.vColorD;
}