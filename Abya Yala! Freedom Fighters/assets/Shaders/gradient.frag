#version 130

uniform sampler2D texture;
uniform vec4 topColor;
uniform vec4 bottomColor;

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec4 texColor = texture2D(texture, uv);
    float gradientFactor = uv.y;
    vec4 gradientColor = mix(bottomColor, topColor, gradientFactor);
    gl_FragColor = texColor * gradientColor;
}