#version 450 core

/////////////
// GLOBALS //
/////////////

layout (std430, binding = 1) buffer MatrixBuffer
{
    mat4 worldMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;

    mat4 textureMatrix1;
    mat4 textureMatrix2;
};

layout(std430, binding = 4) buffer AnimationMatrices
{
    mat4 gBones[256];
};

//////////////
// TYPEDEFS //
//////////////

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec4 Color;
layout (location = 3) in vec2 TexCoord;
layout (location = 4) in vec2 TexCoord2;
layout (location = 5) in uvec4 BoneIndices;
layout (location = 6) in vec4 BoneWeights;

out vertex_output
{
    layout(location = 0) vec4 position;
    layout(location = 1) vec2 tex;
    layout(location = 2) vec2 tex2;
    layout(location = 3) vec4 color;
    layout(location = 4) vec3 UPPos;
    layout(location = 5) vec3 normal;
} VertOut;

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
void main()
{
    vec4 position = vec4(Position, 1.0f);
    vec4 NormalL  = vec4(Normal, 0.0);

    // x World matrix from instance data
    mat4 worldMatrixInstance = worldMatrix;
    mat4 MWorldMatrix = projectionMatrix * viewMatrix;

    {
        mat4 bonematrix  = gBones[BoneIndices[0]] * BoneWeights.x;
             bonematrix += gBones[BoneIndices[1]] * BoneWeights.y;
             bonematrix += gBones[BoneIndices[2]] * BoneWeights.z;
             bonematrix += gBones[BoneIndices[3]] * BoneWeights.w;
    
        position = bonematrix * position;
    }

    position = worldMatrixInstance * vec4(Position, 1.0f);
    position = MWorldMatrix * position;

    gl_Position = position;
    VertOut.position = gl_Position;

    vec4 texCoord1Alt = textureMatrix1 * vec4(TexCoord, 0, 1);
    VertOut.tex = texCoord1Alt.xy / texCoord1Alt.w;

    vec4 texCoord2Alt = textureMatrix2 * vec4(TexCoord2, 0, 1);
    VertOut.tex2 = texCoord2Alt.xy / texCoord2Alt.w;

    VertOut.UPPos    = (worldMatrix * vec4(Position, 1.0f)).xyz;
    VertOut.normal   = (worldMatrix * NormalL).xyz;
    VertOut.color    = Color;
}
