
varying vec2 vTexCoord;

uniform sampler2D uTexture;

void main (void)
{
    gl_FragColor = texture2D(uTexture, vTexCoord) * vec4(0.8, 0.8, 0.8, 1.0);
}
