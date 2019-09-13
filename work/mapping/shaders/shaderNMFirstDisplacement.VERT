#version 330 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// vertex normal in world coordinate
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
out vec4 FragPos;
out vec3 Normal;


void main()
{
	// bool modified = false;
	FragPos = model * vec4(position, 1.0);

	TexCoords = texcoords;

    Normal = mat3(transpose(inverse(model))) * normal;
}