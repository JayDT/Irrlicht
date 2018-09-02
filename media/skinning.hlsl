
float4x4 uMVP;
float4x4 uBone[50];

sampler2D uTexture : register(s0);

struct VS_INPUT
{
	float4 Position : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
	float4 BlendWeight : BLENDWEIGHT;
    float4 BlendIndex : BLENDINDICES;
};

struct VS_PS
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
};

struct PS_OUTPUT
{
	float4 Color : COLOR0;
};

VS_PS vertexMain(VS_INPUT Input)
{
	VS_PS Output;
	
	float4 Position = float4(0.0, 0.0, 0.0, 0.0);
	
	for (int i = 0; i < 4; ++i)
		Position += float4(mul(Input.Position, uBone[int(Input.BlendIndex[i])]) * Input.BlendWeight[i]);

	Output.Position = mul(Position, uMVP);	
	Output.TexCoord = Input.TexCoord;

	return Output;
}

PS_OUTPUT pixelMain(VS_PS Input) 
{
	PS_OUTPUT Output;
	
	Output.Color = tex2D(uTexture, Input.TexCoord) * float4(0.8, 0.8, 0.8, 1.0);
	
	return Output;
}
