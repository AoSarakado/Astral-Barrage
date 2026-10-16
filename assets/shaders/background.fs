#version 330

uniform vec2 resolution;
uniform float time;

out vec4 finalColor;

void main()
{
    vec2 uv = (gl_FragCoord.xy - 0.5*resolution)/resolution.y;
    float distanceFromCenter = length(uv);
    float pulse = 0.5 + 0.5*sin(time*1.2);
    float glow = exp(-5.0*distanceFromCenter)*(0.35 + 0.35*pulse);
    float wave = 0.5 + 0.5*sin(8.0*uv.x + 5.0*uv.y - time*0.8);

    vec3 background = mix(vec3(0.015, 0.025, 0.09), vec3(0.08, 0.04, 0.17), wave);
    vec3 color = background + glow*vec3(0.20, 0.35, 0.75);
    finalColor = vec4(color, 1.0);
}
