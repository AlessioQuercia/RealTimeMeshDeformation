#version 330 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// vertex normal in world coordinate
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// in the fragment shader, we need to calculate also the reflection vector for each fragment
// to do this, we need to calculate in the vertex shader the view direction (in view coordinates) for each vertex, and to have it interpolated for each fragment by the rasterization stage
out vec3 viewPosition;

out vec2 TexCoords;

out vec4 FragPos;

uniform sampler2D displacementMap;  

void main()
{
	TexCoords = texcoords;

	FragPos = vec4(position, 1.0); //vec4(texture(displacementMap, TexCoords).rgb, 1.0); 

	// vertex position in ModelView coordinate (see the last line for the application of projection)
	// when I need to use coordinates in camera coordinates, I need to split the application of model and view transformations from the projection transformations
	vec4 mvPosition = view * FragPos;

	// view direction, negated to have vector from the vertex to the camera
	viewPosition = -mvPosition.xyz;

	// we apply the projection transformation
	gl_Position = projection * mvPosition;
}