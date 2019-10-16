#version 330 core

in vec2 TexCoords;
in vec4 FragPos;
in vec3 Normal;

out vec4 colorFrag;

void main()
{           
    colorFrag = vec4(1.0); //FragPos;
}