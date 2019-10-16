#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec4 FragPos;

uniform sampler2D texture1;

void main()
{    
    FragColor = FragPos;
}