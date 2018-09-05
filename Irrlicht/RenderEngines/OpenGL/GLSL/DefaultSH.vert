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

//////////////
// TYPEDEFS //
//////////////

layout(location = 0) in vec3 Position;

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
    mat4 MWorldMatrix = projectionMatrix * viewMatrix;
    vec4 position = worldMatrix * vec4(Position, 1.0f);

    position = MWorldMatrix * position;

    gl_Position = position;
    VertOut.position = gl_Position;
    VertOut.color    = vec4(0, 0, 0, 0);
}
