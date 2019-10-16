#version 330 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// vertex normal in world coordinate
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 hittingPoint;
uniform vec3 hittingDirection;

out vec2 TexCoords;
out vec4 FragPos;
out vec3 Normal;

const float range = 0.5f;
const float power = 0.05f;
const float max_magnitude = 0.3f;

vec4 explode(vec4 position, vec3 direction, vec3 normal, float magnitude)
{
    direction = direction * magnitude;
    // Normal = mat3(transpose(inverse(model))) * normalize (-direction);
    return position + vec4(direction, 0.0);
}

float getDistance(vec3 point1, vec3 point2)
{
	return sqrt( pow(point1.x - point2.x, 2) + pow(point1.y - point2.y, 2) + pow(point1.z - point2.z, 2) );
}


void main()
{
	FragPos = model * vec4(position, 1.0);

	float distance = getDistance(hittingPoint, FragPos.xyz);

	if (distance < range)
	{
		float magnitude = power/ distance;
		magnitude = min(magnitude, max_magnitude);
		FragPos = explode(FragPos, hittingDirection, -normal, magnitude);
		FragPos = inverse(model) * FragPos;
	}
	else
	{
		FragPos = vec4(position, 1.0);
	}
}