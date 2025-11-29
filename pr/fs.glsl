#version 400

in vec3 color;
in vec3 world_pos;

out vec4 frag_color;

void main()
{
    vec3 base = color;

    frag_color = vec4(base, 1.0);
}
