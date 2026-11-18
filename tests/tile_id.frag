#version 330 core
uniform vec3 tile_color;
out vec4 frag_color;
void main() { frag_color = vec4(tile_color, 1.0); }
