#version 330 core

out vec2 screen_position;

void main()
{
    vec2 corner = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    screen_position = corner * 2.0 - 1.0;
    gl_Position = vec4(screen_position, 1.0, 1.0);
}
