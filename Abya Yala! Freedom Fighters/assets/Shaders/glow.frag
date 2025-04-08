uniform float time;
uniform vec4 baseColor;

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(800.0, 600.0); 
    float shimmer = 0.5 + 0.5 * sin(time * 10.0 + uv.x * 10.0);
    vec4 color = baseColor * shimmer;
    gl_FragColor = color;
}