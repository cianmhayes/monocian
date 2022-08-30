#version 330 core
out vec4 FragColor;
in vec3 normal_vector;
in vec3 fragment_pos;
in vec2 texture_coords;
uniform vec3 view_position;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
  
uniform Material material;
uniform Light light;

void main()
{
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, texture_coords));

    vec3 norm = normalize(normal_vector);
    vec3 light_dir = normalize(light.position - fragment_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texture_coords));

    vec3 view_dir = normalize(view_position - fragment_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, texture_coords));

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}