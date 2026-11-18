#version 330 core

uniform sampler2D sky_image;
uniform mat4 inverse_projection;
uniform mat4 inverse_view;
in vec2 screen_position;
out vec4 frag_color;

void main()
{
    vec4 ray = inverse_projection * vec4(screen_position, 1.0, 1.0);
    vec3 direction = normalize(mat3(inverse_view) * ray.xyz);
    // NASA's celestial map: north at the top, RA zero at the centre,
    // and right ascension increasing to the left.
    const float pi = 3.141592653589793;
    vec2 uv = vec2(fract(0.5 - atan(direction.y, direction.x) / (2.0 * pi)),
                   acos(clamp(direction.z, -1.0, 1.0)) / pi);
    frag_color = vec4(texture(sky_image, uv).rgb * 0.75, 1.0);
}
