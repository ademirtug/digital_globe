
#version 330 core
struct ulight {
    vec3 position;  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float linear;
    float quadratic;
    float constant;
};

struct umaterial  {
    sampler2D diffuse;
    vec3 specular;
    float shininess;
}; 

uniform ulight light;
uniform umaterial material;
uniform vec3 view_pos;

in vec2 tex_coord;
in vec3 normal;
in vec3 frag_position;

out vec4 frag_color;


void main()
{
    // ambient
    vec3 ambient = light.ambient * texture(material.diffuse, tex_coord).rgb;
  	
    // diffuse 
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light.position - frag_position);
    vec3 diffuse = light.diffuse * max(dot(-light_dir, normal), 0.0) * texture(material.diffuse, tex_coord).rgb;  
    
    // specular
    vec3 view_dir = normalize(view_pos - frag_position);
    vec3 reflect_dir = reflect(-light_dir, norm);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    vec3 specular = light.specular * material.specular * pow(max(dot(normal, halfway_dir), 0.0), 32.0);  

    //attenuation
    float distance = length(light.position - frag_position);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 

    frag_color = vec4((ambient + diffuse + specular) * (1+attenuation), 1.0);
}
