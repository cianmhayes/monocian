#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texture_coords;
out vec3 normal_vector;
out vec3 fragment_pos;
out vec2 texture_coords;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main()
{
    fragment_pos = vec3(model * vec4(a_pos, 1.0));
    normal_vector = mat3(transpose(inverse(model))) * a_normal;
    texture_coords = a_texture_coords;
    gl_Position = projection * view * vec4(fragment_pos, 1.0);
}