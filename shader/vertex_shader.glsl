#version 330 core 

layout (location = 0) in vec3 square;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aOffset;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main()
{

    FragPos = vec3(model * vec4(square, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;

    float scale = 0.1f;
    gl_Position = projection * view * vec4(scale * square + aOffset, 1.0);
}
