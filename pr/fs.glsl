#version 400

in vec3 color;
in vec3 world_pos;

out vec4 frag_color;

uniform float u_time;

void main()
{
    // базовый цвет из модели
    vec3 base = color;

    // немного "живости" Ч лЄгка€ модул€ци€ цвета по времени
    float pulse = 0.05 * sin(u_time * 2.0);
    base = base + pulse;

    frag_color = vec4(base, 1.0);
}
