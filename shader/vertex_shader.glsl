#version 330 core 

layout (location = 0) in vec3 squareCenter;
layout (location = 1) in vec3 xyz;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    float scale = 0.1f;
    vec3 square = squareCenter + xyz;
    gl_Position = projection * view * vec4(scale * square, 1.0);
}
