// part of the Irrlicht Engine Shader example.
// These simple Direct3D11 pixel and vertex shaders will be loaded by the shaders
// example. Please note that these example shaders don't do anything really useful. 
// They only demonstrate that shaders can be used in Irrlicht.

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
cbuffer cbParams : register(c0)
{
	float4x4 mWorldViewProj;
	float4x4 mInvWorld;       // Inverted world matrix
	float4x4 mTransWorld;     // Transposed world matrix
	float4 mLightColor;       // Light color
	float3 mLightPos;         // Light position
};

struct VS_INPUT
{
	float4 vPosition : POSITION;
	float3 vNormal   : NORMAL;
	float2 texCoord  : TEXCOORD0;
};

// Vertex shader output structure
struct VS_OUTPUT
{
	float4 Position   : SV_Position;	// vertex position 
	float4 Diffuse    : COLOR0;			// vertex diffuse color
	float2 TexCoord   : TEXTURE0;		// tex coords
};

VS_OUTPUT vertexMain( VS_INPUT input )
{
	VS_OUTPUT Output;

	// transform position to clip space 
	Output.Position = mul(input.vPosition, mWorldViewProj);

	// transform normal 
	float4 normal = mul(float4(input.vNormal,0.0), mInvWorld);
	
	// renormalize normal 
	normal = normalize(normal);
	
	// position in world coodinates
	float4 worldpos = mul(mTransWorld, input.vPosition);
	
	// calculate light vector, vtxpos - lightpos
	float3 lightVector = worldpos.xyz - mLightPos;
	
	// normalize light vector 
	lightVector = normalize(lightVector);
	
	// calculate light color 
	float3 tmp = dot(-lightVector, normal.xyz);
	tmp = lit(tmp.x, tmp.y, 1.0).xyz;
	
	tmp = mLightColor.xyz * tmp.y;
	
	Output.Diffuse = float4(tmp.x, tmp.y, tmp.z, 0);
	Output.TexCoord = input.texCoord;
	
	return Output;
}

Texture2D myTexture : register(t0);
SamplerState st : register(s0);
	
float4 pixelMain( VS_OUTPUT input ) : SV_Target
{ 
	float4 Output;

	float4 col = myTexture.Sample( st, input.TexCoord );  // sample color map
	
	// multiply with diffuse and do other senseless operations
	Output = input.Diffuse * col;
	Output *= 4.0;
	
	return Output;
}

