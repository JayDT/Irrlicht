#version 450 core

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

//////////////
// TYPEDEFS //
//////////////

in vertex_output
{
    layout(location = 0) vec4 position;
    layout(location = 1) vec2 tex;
    layout(location = 2) vec2 tex2;
    layout(location = 3) vec4 color;
    layout(location = 4) vec3 UPPos;
    layout(location = 5) vec3 normal;
} _input;

out layout(location = 0) vec4 FragColor;

/////////////
// GLOBALS //
/////////////

struct Light
{
    vec3 Position;
    vec3 Atten;
    vec4 Diffuse;
    vec4 Specular;
    vec4 Ambient;
    float  Range;
    float  Falloff;
    float  Theta;
    float  Phi;
    int    Type;
};

struct Material
{
    vec4    Ambient;
    vec4    Diffuse;
    vec4    Specular;
    vec4    Emissive;
    float   Shininess;
    int     Type;
    bool    Lighted;
    bool    Fogged;
};

layout(std430, binding = 2) buffer PixelConstats
{
    int         g_nTexture;
    int         g_bAlphaTest;
    int         g_iLightCount; // Number of lights enabled
    float       g_fAlphaRef;
    vec4        g_fogColor;
    // x -> fogStart
    // y -> fotEnd
    // z -> farClip
    // w -> fogType
    vec4        g_fogParams;
    vec4        g_eyePositionVert;
    Material    g_material;    
    Light       g_lights[MAX_LIGHTS];
};

struct LightConstantData
{
    vec3 DiffuseLight;
    vec3 AmbientLight;
    vec3 SpecularLight;
};

layout(binding=0) uniform sampler2D shaderTexture;
layout(binding=3) uniform sampler2D shaderTexture2;

float saturate(float rgb)
{
    return clamp(rgb, 0.0, 1.0);
}

vec3 saturate(vec3 rgb)
{
    return clamp(rgb, 0.0, 1.0);
}

vec4 saturate(vec4 rgb)
{
    return clamp(rgb, 0.0, 1.0);
}

vec4 sinusInterpolate(vec4 src, vec4 dst, float pct)
{
    float sinval = sin(pct * 3.1415926 / 2.0f);
    return sinval * dst + (1 - sinval) * src;
}

void buildConstantLighting(vec3 normal, vec3 worldPos, inout LightConstantData ret)
{
    vec3 lightDir = vec3(0.0, -0.5, 0.0); //lights[0].Position - cameraTarget

    //ret.DiffuseLight = g_material.Diffuse.rgb;
    ret.AmbientLight = g_material.Ambient.rgb; 

    for (int i = 0; i < 1; ++i)
    {
        vec3 V = g_lights[i].Position.xyz - worldPos;
        float d = length(V);

        //if (d > g_lights[i].Range)
        //    continue;

        ret.AmbientLight += g_lights[i].Ambient.rgb;
        ret.AmbientLight *= 0.5f;
        vec3 finalDiffuse = g_lights[i].Diffuse.rgb;

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
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
void main()
{
    vec4 textureColor;
    if (g_nTexture > 0)
    {
        vec4 textureColor2;
        textureColor = texture(shaderTexture, _input.tex);
    
        switch(g_material.Type)
        {
            case EMT_SOLID:
                textureColor = (textureColor * _input.color.bgra);
                break;
            //case EMT_ONETEXTURE_BLEND:
            //    textureColor.rgb = (textureColor.rgb * _input.color.bgr);
            //    break;
            case EMT_TRANSPARENT_ALPHA_CHANNEL:
                textureColor = (textureColor * _input.color.bgra);
                break;
            case EMT_TRANSPARENT_ADD_COLOR: // TODO
                textureColor.rgb = (textureColor.rgb * _input.color.bgr);
                break;
            case EMT_TRANSPARENT_VERTEX_ALPHA:
                textureColor = (textureColor * _input.color.bgra);
                break;
            case EMT_SOLID_2_LAYER:
            case EMT_LIGHTMAP:
            case EMT_LIGHTMAP_LIGHTING:
                textureColor2 = texture(shaderTexture2, _input.tex2);

                textureColor = (textureColor * _input.color.bgra);
	            textureColor = textureColor * textureColor2;
                break;
            case EMT_TRANSPARENT_REFLECTION_2_LAYER:
            case EMT_REFLECTION_2_LAYER:
                // ToDo: This is wrong
                vec3 V = normalize(g_eyePositionVert.xyz);
                //input.normal.w = 0;
                vec3 normal = normalize(_input.normal);

                vec3 reflectVec = normalize(reflect(V, normal));
                textureColor2 = texture(shaderTexture2, reflectVec.xy);

                textureColor = (textureColor * _input.color.bgra);
	            textureColor = textureColor * textureColor2;
                break;
            case EMT_LIGHTMAP_M2:
            case EMT_LIGHTMAP_LIGHTING_M2:
                textureColor2 = texture(shaderTexture2, _input.tex2);

                textureColor = (textureColor * _input.color.bgra);
	            textureColor = (textureColor * textureColor2) * 2.0;
                break;
            case EMT_LIGHTMAP_M4:
            case EMT_LIGHTMAP_LIGHTING_M4:
                textureColor2 = texture(shaderTexture2, _input.tex2);

                textureColor = (textureColor * _input.color.bgra);
	            textureColor = (textureColor * textureColor2) * 4.0;
                break;
        };

        if (g_bAlphaTest != 0 && g_fAlphaRef > textureColor.a)
            discard;

        if (g_material.Lighted)
        {
            LightConstantData lightData;
            buildConstantLighting(_input.normal, _input.UPPos, lightData);
            textureColor.rgb *= lightData.DiffuseLight + lightData.AmbientLight /*+ lightData.SpecularLight*/;
        }

        if (g_material.Fogged)
        {
            float distance = length(_input.position.xyz - g_eyePositionVert.xyz);
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
               textureColor = _input.color.bgra;
               break;
           case EMT_SOLID_2_LAYER:
               textureColor = (g_material.Ambient.bgra * _input.color.bgra);
               break;
       };
    }
     
    FragColor = textureColor;
}
