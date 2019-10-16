#version 330 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// vertex normal in world coordinate
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 FragPos;
out vec2 texCoords;

void main()
{
	texCoords = texcoords;

	FragPos = model * vec4(position, 1.0);

	// vertex position in ModelView coordinate (see the last line for the application of projection)
	// when I need to use coordinates in camera coordinates, I need to split the application of model and view transformations from the projection transformations
	vec4 mvPosition = view * FragPos;

	// we apply the projection transformation
	gl_Position = projection * mvPosition;
}