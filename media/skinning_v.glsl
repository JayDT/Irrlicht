
varying vec2 vTexCoord;

attribute vec4 inBlendWeight;
attribute vec4 inBlendIndex;

uniform mat4 uMVP;
uniform mat4 uBone[50];

void main(void)
{
	vec4 Position = vec4(0.0, 0.0, 0.0, 0.0);

	for (int i = 0; i < 4; ++i)
		Position += vec4(uBone[int(inBlendIndex[i])] * gl_Vertex * inBlendWeight[i]);

	gl_Position = uMVP * Position;		
	vTexCoord = gl_MultiTexCoord0.xy;
}
