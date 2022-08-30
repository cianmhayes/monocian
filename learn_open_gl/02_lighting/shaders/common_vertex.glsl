#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
out vec3 normal_vector;
out vec3 fragment_pos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main()
{
    fragment_pos = vec3(model * vec4(a_pos, 1.0));
    normal_vector = mat3(transpose(inverse(model))) * a_normal;
    gl_Position = projection * view * vec4(fragment_pos, 1.0);
}