#version 330 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// vertex normal in world coordinate
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 impactPoints[600];
uniform vec3 hittingDirections[600];

// the transformed normal (in view coordinate) is set as an output variable, to be "passed" to the fragment shader
// this means that the normal values in each vertex will be interpolated on each fragment created during rasterization between two vertices
out vec3 Normal;

// in the fragment shader, we need to calculate also the reflection vector for each fragment
// to do this, we need to calculate in the vertex shader the view direction (in view coordinates) for each vertex, and to have it interpolated for each fragment by the rasterization stage
out vec3 viewPosition;

out vec4 FragPos;

out vec2 TexCoords;

const float range = 0.5f;
const float power = 0.05f;
const float max_magnitude = 0.3f;

vec4 explode(vec4 position, vec3 direction, vec3 normal, float magnitude)
{
    direction = direction * magnitude;
    Normal = mat3(transpose(inverse(model))) * -normalize(normal + direction);
    return position + vec4(direction, 0.0);
}

float getDistance(vec3 point1, vec3 point2)
{
	return sqrt( pow(point1.x - point2.x, 2) + pow(point1.y - point2.y, 2) + pow(point1.z - point2.z, 2) );
}


void main()
{
	bool modified = false;
	FragPos = model * vec4(position, 1.0);

	vec4 worldPos = FragPos;

	TexCoords = texcoords;

	for (int i = 0; i<600; i++)
    {
    	float distance = getDistance(impactPoints[i], vec3(worldPos.x, worldPos.y, worldPos.z));

    	if (distance < range)
    	{
    		float magnitude = power/ distance;
    		magnitude = min(magnitude, max_magnitude);
			worldPos = explode(worldPos, hittingDirections[i], -normal, magnitude);
			FragPos = worldPos;
			modified = true;
		    // break;
    	}
    }

    if (!modified)
	   	// transformations are applied to the normal
		Normal = mat3(transpose(inverse(model))) * normal;

	// vertex position in ModelView coordinate (see the last line for the application of projection)
	// when I need to use coordinates in camera coordinates, I need to split the application of model and view transformations from the projection transformations
	vec4 mvPosition = view * worldPos;

	// view direction, negated to have vector from the vertex to the camera
	viewPosition = -mvPosition.xyz;

	// we apply the projection transformation
	gl_Position = projection * mvPosition;
}