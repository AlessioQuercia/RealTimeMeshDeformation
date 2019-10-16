#version 330 core

in vec4 FragPos;
in vec2 texCoords;

// output shader variable
out vec4 colorFrag;

uniform sampler2D texture1;

void main()
{    
    colorFrag = texture(texture1, texCoords);
}