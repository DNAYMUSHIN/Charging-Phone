#version 400

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;

out vec3 color;
out vec3 world_pos;

uniform mat4 MVP;
uniform mat4 ModelMat;

void main()
{
    color = vertex_color;
    world_pos = (ModelMat * vec4(vertex_position, 1.0)).xyz;
    gl_Position = MVP * vec4(vertex_position, 1.0);
}
