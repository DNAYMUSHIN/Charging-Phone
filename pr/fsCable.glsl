#version 400

in vec3 color;
in vec3 world_pos;
out vec4 frag_color;

uniform float u_time;

void main()
{
    vec3 base = color;

    float coord = world_pos.x * 3.0 + world_pos.y * 2.0 + world_pos.z * 3.0;
    float stripe = 0.5 + 0.5 * sin((coord + u_time) * 20.0);
    float band = smoothstep(0.7, 0.72, stripe);

    vec3 glow = vec3(1.0, 0.8, 0.3) * (0.5 * band);

    vec3 outc = base + glow;

    frag_color = vec4(outc, 1.0);
}
