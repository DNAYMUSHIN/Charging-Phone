#version 400

in vec3 color;
in vec3 world_pos;
out vec4 frag_color;

uniform float u_time;

void main()
{
    // базовый цвет от вершин
    vec3 base = color;

    // вычислим анимированную "полоску тока" вдоль кабеля:
    // используем комбинированную координату вдоль кабеля (x + z),
    // и смещаем её во времени, получая бегущую полосу
    float coord = world_pos.x * 3.0 + world_pos.y * 2.0 + world_pos.z * 3.0;
    float t = u_time * 3.0;
    float stripe = 0.5 + 0.5 * sin((coord - t) * 20.0); // колебание
    // делаем резкую полоску:
    float band = smoothstep(0.7, 0.82, stripe) * 1.0;

    // небольшие случайные искры (временное псевдо-шумоподобное)
    float spark = smoothstep(0.95, 1.0, fract(sin(dot(world_pos.xy ,vec2(12.9898,78.233))) * 43758.5453 + u_time*10.0));

    vec3 glow = vec3(1.0, 0.8, 0.3) * (0.5 * band + 0.7 * spark);

    vec3 outc = base + glow;

    frag_color = vec4(outc, 1.0);
}
