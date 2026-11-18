#version 330 core

struct umaterial {
    sampler2D diffuse;
};

uniform umaterial material;
in vec2 tex_coord;
out vec4 frag_color;

void main()
{
    frag_color = vec4(texture(material.diffuse, tex_coord).rgb, 1.0);
}
