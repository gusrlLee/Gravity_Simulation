#version 330 core 

layout (location = 0) in vec3 square;
layout (location = 1) in vec3 aOffset;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    float scale = 0.1f;
    gl_Position = projection * view * vec4(scale * square + aOffset, 1.0);
}
