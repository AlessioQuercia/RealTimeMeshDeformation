#version 330 core

in vec2 TexCoords;
in vec4 FragPos;
in vec3 Normal;

out vec3 colorFrag;

void main()
{           
    colorFrag = normalize(Normal);
}