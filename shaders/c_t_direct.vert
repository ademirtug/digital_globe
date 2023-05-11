#version 330 core
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 normal;
out vec3 frag_position;
out vec2 tex_coord;

void main()
{
    frag_position = vec3(model * vec4(in_pos, 1.0));
    normal = in_normal;

    gl_Position = projection * view * model * vec4(in_pos, 1.0);
    tex_coord = in_tex_coord;
}

