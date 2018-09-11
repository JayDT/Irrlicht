/////////////
// DEFINES //
/////////////

#define MAX_LIGHTS                                      4

// Material types
#define EMT_SOLID                                       0
#define EMT_SOLID_2_LAYER                               1
#define EMT_LIGHTMAP                                    2
#define EMT_LIGHTMAP_ADD                                3
#define EMT_LIGHTMAP_M2                                 4
#define EMT_LIGHTMAP_M4                                 5
#define EMT_LIGHTMAP_LIGHTING                           6
#define EMT_LIGHTMAP_LIGHTING_M2                        7
#define EMT_LIGHTMAP_LIGHTING_M4                        8
#define EMT_DETAIL_MAP                                  9
#define EMT_SPHERE_MAP                                  10
#define EMT_REFLECTION_2_LAYER                          11
#define EMT_TRANSPARENT_ADD_COLOR                       12
#define EMT_TRANSPARENT_ALPHA_CHANNEL                   13
#define EMT_TRANSPARENT_ALPHA_CHANNEL_REF               14
#define EMT_TRANSPARENT_VERTEX_ALPHA                    15
#define EMT_TRANSPARENT_REFLECTION_2_LAYER              16
#define EMT_ONETEXTURE_BLEND                            23

/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer : register(b0)
{
    row_major matrix worldMatrix;
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;
    //row_major matrix WVP;

    row_major float4x4 textureMatrix1;
    row_major float4x4 textureMatrix2;
};

//////////////
// TYPEDEFS //
//////////////

struct VS_INPUT_SHADOW
{
    float3 Position : POSITION0;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float2 tex2 : TEXCOORD1;
    float4 color : COLOR0;
    float3 normal : NORMAL0;
    float3 UPPos : POSITION0;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader (Shadow)
////////////////////////////////////////////////////////////////////////////////
PS_INPUT vs_main_sh(VS_INPUT_SHADOW input)
{
    float4 position = mul(float4(input.Position, 1.0f), worldMatrix);
    position = mul(position, mul(viewMatrix, projectionMatrix));

    PS_INPUT output = (PS_INPUT) 0;
    output.position = position;
    output.color = float4(0, 0, 0, 0);
    
    return output;
}

//////////////
// TYPEDEFS //
//////////////

struct VS_INPUT
{
    float3 Position  : POSITION0;
    float3 Normal    : NORMAL0;
    float4 color     : COLOR0;
    float2 texCoord  : TEXCOORD0;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PS_INPUT vs_main(VS_INPUT input)
{
    float4 position = mul(float4(input.Position, 1.0f), worldMatrix);
    position = mul(position, mul(viewMatrix, projectionMatrix));

    PS_INPUT output = (PS_INPUT)0;
    output.position = position;

    float4 texCoord1Alt = mul(float4(input.texCoord, 0, 1), textureMatrix1);
    output.tex = texCoord1Alt.xy / texCoord1Alt.w;

    float4 texCoord2Alt = mul(float4(input.texCoord, 0, 1), textureMatrix2);
    output.tex2 = texCoord2Alt.xy / texCoord2Alt.w;

    output.normal   = mul(input.Normal, (float3x3)worldMatrix);
    output.color    = input.color;
    output.UPPos = mul(float4(input.Position, 1.0f), worldMatrix).xyz;
    
    return output;
}

//////////////
// TYPEDEFS //
//////////////

struct VS_INPUT_T2
{
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
    float4 color : COLOR0;
    float2 texCoord : TEXCOORD0;
    float2 texCoord2 : TEXCOORD1;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader T2
////////////////////////////////////////////////////////////////////////////////
PS_INPUT vs_main_t2(VS_INPUT_T2 input)
{
    float4 position = mul(float4(input.Position, 1.0f), worldMatrix);
    position = mul(position, mul(viewMatrix, projectionMatrix));

    PS_INPUT output = (PS_INPUT) 0;
    output.position = position;

    float4 texCoord1Alt = mul(float4(input.texCoord, 0, 1), textureMatrix1);
    output.tex = texCoord1Alt.xy / texCoord1Alt.w;

    float4 texCoord2Alt = mul(float4(input.texCoord2, 0, 1), textureMatrix2);
    output.tex2 = texCoord2Alt.xy / texCoord2Alt.w;

    output.normal   = mul(input.Normal, (float3x3) worldMatrix);
    output.color    = input.color;
    output.UPPos = mul(float4(input.Position, 1.0f), worldMatrix).xyz;
    
    return output;
}

//////////////
// TYPEDEFS //
//////////////

cbuffer AnimationMatrices : register(b1)
{
    row_major float4x4 gBones[256];
};

struct VS_INPUT_SK
{
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
    float4 color : COLOR0;
    float2 texCoord : TEXCOORD0;
    float2 texCoord2 : TEXCOORD1;
    int4 BoneIndices : BLENDINDICES; // Note this is UBYTE4  in the source data
    float4 BoneWeights : BLENDWEIGHT; // Note this as UBYTE4N in the source data
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader Skinning
////////////////////////////////////////////////////////////////////////////////
PS_INPUT vs_main_sk(VS_INPUT_SK input)
{
    float4x4 worldMatrixInstance = worldMatrix;

    {
        float4x4 bonematrix =  mul(gBones[input.BoneIndices[0]], input.BoneWeights.x);
                 bonematrix += mul(gBones[input.BoneIndices[1]], input.BoneWeights.y);
                 bonematrix += mul(gBones[input.BoneIndices[2]], input.BoneWeights.z);
                 bonematrix += mul(gBones[input.BoneIndices[3]], input.BoneWeights.w);

        worldMatrixInstance = mul(bonematrix, worldMatrixInstance);
    }

    float4 position = mul(float4(input.Position, 1.0f), worldMatrixInstance);
    position = mul(position, mul(viewMatrix, projectionMatrix));

    PS_INPUT output = (PS_INPUT) 0;
    output.position = position;

    float4 texCoord1Alt = mul(float4(input.texCoord, 0, 1), textureMatrix1);
    output.tex = texCoord1Alt.xy / texCoord1Alt.w;

    float4 texCoord2Alt = mul(float4(input.texCoord2, 0, 1), textureMatrix2);
    output.tex2 = texCoord2Alt.xy / texCoord2Alt.w;

    output.normal = mul(input.Normal, (float3x3) worldMatrixInstance);
    output.color = input.color;
    output.UPPos = mul(float4(input.Position, 1.0f), worldMatrix).xyz;
    
    return output;
}

//////////////
// TYPEDEFS //
//////////////

struct VS_INPUT_BT
{
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
    float4 color : COLOR0;
    float2 texCoord : TEXCOORD0;
    float3 Binormal : BINORMAL0;
    float3 Tangent : TANGENT;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader Tangent (ToDo: not implement yet)
////////////////////////////////////////////////////////////////////////////////
PS_INPUT vs_main_bt(VS_INPUT_BT input)
{
    float4 position = mul(float4(input.Position, 1.0f), worldMatrix);
    position = mul(position, mul(viewMatrix, projectionMatrix));

    PS_INPUT output = (PS_INPUT) 0;
    output.position = position;

    float4 texCoord1Alt = mul(float4(input.texCoord, 0, 1), textureMatrix1);
    output.tex = texCoord1Alt.xy / texCoord1Alt.w;

    float4 texCoord2Alt = mul(float4(input.texCoord, 0, 1), textureMatrix2);
    output.tex2 = texCoord2Alt.xy / texCoord2Alt.w;

    output.normal = mul(input.Normal, (float3x3) worldMatrix);
    output.color = input.color;
    output.UPPos = mul(float4(input.Position, 1.0f), worldMatrix).xyz;

    return output;
}

/////////////
// GLOBALS //
/////////////

struct Light
{
    float4 Position;
    float4 Atten;
    float4 Diffuse;
    float4 Specular;
    float4 Ambient;
    float  Range;
    float  Falloff;
    float  Theta;
    float  Phi;
    int    Type;
};

struct Material
{
    float4    Ambient;
    float4    Diffuse;
    float4    Specular;
    float4    Emissive;
    float     Shininess;
    int       Type;
    bool      Lighted;
    bool      Fogged;
};

cbuffer PixelConstats : register(b0)
{
    int         g_nTexture;
    int         g_bAlphaTest;
    int         g_iLightCount; // Number of lights enabled
    float       g_fAlphaRef;
    float4      g_fogColor;
    // x -> fogStart
    // y -> fotEnd
    // z -> farClip
    // w -> fogType
    float4      g_fogParams;
    float4      g_eyePositionVert;
    Material    g_material;
    Light       g_lights[MAX_LIGHTS];
};

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

Texture2D shaderTexture2 : register(t1);
SamplerState SampleType2 : register(s1);

struct PixelOutput
{
    float4 FragColor    : SV_TARGET0;
    //float4 Normal     : SV_TARGET1;
    //float4 Depth     : SV_TARGET2;
    //float  FragDepth : DEPTH0;
};

struct LightConstantData
{
    float3 DiffuseLight;
    float3 AmbientLight;
    float3 SpecularLight;
};

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////

LightConstantData buildConstantLighting(float3 normal, float3 worldPos)
{
    LightConstantData ret = (LightConstantData) 0;
    float3 lightDir = float3(0.0, -0.5, 0.0); //lights[0].Position - cameraTarget

    //ret.DiffuseLight = g_material.Diffuse.rgb;
    ret.AmbientLight = g_material.Ambient.rgb; 

    for (int i = 0; i < 1; ++i)
    {
        float3 V = g_lights[i].Position.xyz - worldPos;
        float d = length(V);

        if (d > g_lights[i].Range)
            continue;

        float3 finalDiffuse = g_lights[i].Ambient.rgb + g_lights[i].Diffuse.rgb;

        if (g_lights[i].Type == 0)
        {
            float intensityRange = g_lights[i].Range - d;

            finalDiffuse *= (g_lights[i].Atten[0] + (g_lights[i].Atten[1] * intensityRange)) + (g_lights[i].Atten[2] * (intensityRange * intensityRange));
        }
        else
        {
            float intensityRange = g_lights[i].Range - d;

            float howMuchLight = dot(V, normal);

            if (howMuchLight > 0.0f)
            {
                finalDiffuse *= (g_lights[i].Atten[0] + (g_lights[i].Atten[1] * intensityRange)) + (g_lights[i].Atten[2] * (intensityRange * intensityRange));
                //finalDiffuse *= pow(max(dot(-V, lightDir), 0.0f), g_lights[i].Theta);
            }
        }

        
        ret.DiffuseLight = saturate(ret.DiffuseLight + finalDiffuse);
    }

    ret.AmbientLight = saturate(ret.AmbientLight);

    //float Si = 0.5f;
    //float3 Sc = g_lights[0].Specular.xyz;
    //float3 V = normalize(eyePosition.xyz - worldPos); // VertexToEye
    //float3 R = normalize(reflect(lightDir, norm));
    //float Sfactor = dot(V, R);
    //if (Sfactor > 0)
    //{
    //    Sfactor = pow(abs(Sfactor), 32);
    //    ret.SpecularLight = Si * Sc * Sfactor;
    //}

    return ret;
}

PixelOutput ps_main(PS_INPUT input)
{
    float4 textureColor;
    if (g_nTexture > 0)
    {
        float4 textureColor2;
        if (g_material.Type == EMT_SPHERE_MAP)
        {
            float3 u = normalize(input.position.xyz);
            float3 n = normalize(input.normal);
            float3 r = reflect(u, n);
            float m = 2.0 * sqrt(r.x * r.x + r.y * r.y + (r.z + 1.0) * (r.z + 1.0));
            float2 tex;
            tex.x = r.x / m + 0.5;
            tex.y = r.y / m + 0.5;

            textureColor = shaderTexture.Sample(SampleType, tex);
        }
        else
        {
            textureColor = shaderTexture.Sample(SampleType, input.tex);
        }

        // check material type, and add color operation correctly
        [branch]
        switch (g_material.Type)
        {
            case EMT_SOLID:
                textureColor = (textureColor * input.color.bgra);
                break;
            //case EMT_ONETEXTURE_BLEND:
            //    textureColor = (textureColor * input.color.bgra);
            //    break;
            case EMT_TRANSPARENT_ALPHA_CHANNEL:
                textureColor = (textureColor * input.color.bgra);
                break;
            case EMT_TRANSPARENT_ADD_COLOR: // TODO
                textureColor.rgb = (textureColor.rgb * input.color.bgr);
                break;
            case EMT_TRANSPARENT_VERTEX_ALPHA:
                textureColor = (textureColor * input.color.bgra);
                break;
            case EMT_TRANSPARENT_ALPHA_CHANNEL_REF:
                textureColor = (textureColor * input.color.bgra);
                break;
            case EMT_SOLID_2_LAYER:
            case EMT_LIGHTMAP:
            case EMT_LIGHTMAP_LIGHTING:
                textureColor2 = shaderTexture2.Sample(SampleType2, input.tex2);

                textureColor = (textureColor * input.color.bgra);
                textureColor = textureColor * textureColor2;
                break;
            case EMT_TRANSPARENT_REFLECTION_2_LAYER:
            case EMT_REFLECTION_2_LAYER:
                // ToDo: This is wrong
                float3 V = normalize(g_eyePositionVert.xyz);
                //input.normal.w = 0;
                float3 normal = normalize(input.normal);

                float3 reflectVec = normalize(reflect(V, normal));
                textureColor2 = shaderTexture2.Sample(SampleType2, reflectVec.xy);

                textureColor = (textureColor * input.color.bgra);
                textureColor = textureColor * textureColor2;
                break;
            case EMT_LIGHTMAP_M2:
            case EMT_LIGHTMAP_LIGHTING_M2:
                textureColor2 = shaderTexture2.Sample(SampleType2, input.tex2);

                textureColor = (textureColor * input.color.bgra);
                textureColor = (textureColor * textureColor2) * 2.0;
                break;
            case EMT_LIGHTMAP_M4:
            case EMT_LIGHTMAP_LIGHTING_M4:
                textureColor2 = shaderTexture2.Sample(SampleType2, input.tex2);

                textureColor = (textureColor * input.color.bgra);
                textureColor = (textureColor * textureColor2) * 4.0;
                break;
        };

        //[branch]
        if (g_bAlphaTest > 0 && g_fAlphaRef > textureColor.a)
            clip(-1);

        if (g_material.Lighted)
        {
            LightConstantData lightData = buildConstantLighting(input.normal, input.UPPos);
            textureColor.rgb *= lightData.DiffuseLight + lightData.AmbientLight /*+ lightData.SpecularLight*/;
        }

        if (g_material.Fogged)
        {
            float distance = length(input.UPPos.xyz - g_eyePositionVert.xyz);
            float fogDepth = distance - g_fogParams.x;
            fogDepth /= (g_fogParams.y - g_fogParams.x);
            float fog = 1.0f - pow(saturate(fogDepth), 1.5);

            textureColor.rgb = (1.0 - fog) * g_fogColor.rgb + fog * textureColor.rgb;
        }
    }
    else
    {
        switch (g_material.Type)
        {
            case EMT_SOLID:
            default:
                textureColor = input.color.bgra;
                break;
            case EMT_SOLID_2_LAYER:
                textureColor = (g_material.Ambient * input.color.bgra);
                break;
        };
    }
    
    PixelOutput pixelData = (PixelOutput)0;
    pixelData.FragColor = saturate(textureColor);

    return pixelData;
}

PixelOutput ps_main_sh(PS_INPUT input)
{
        
    // Get the depth value of the pixel by dividing the Z pixel depth by the homogeneous W coordinate.
    //float depthValue = input.position.z / input.position.w;

    PixelOutput pixelData = (PixelOutput) 0;
    pixelData.FragColor = float4(0, 0, 0, 0);

    return pixelData;
}
