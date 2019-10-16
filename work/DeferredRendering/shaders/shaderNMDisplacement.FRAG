#version 330 core

in vec2 TexCoords;
in vec4 FragPos;
in vec3 Normal;

layout(location = 0) out vec3 color;

void main()
{           
    color = FragPos.xyz;
}