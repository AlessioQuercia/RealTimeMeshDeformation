#version 330 core

// vertex position in world coordinates
in vec3 position;
// vertex normal in world coordinate
in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 hitPoint;
uniform vec3 hitDirection;

out vec4 FragPos;
out vec3 Normal;
out vec3 outValue;
out vec3 outValue2;

// const float range = 0.5f;
// const float power = 0.55f;
// const float max_magnitude = 0.9f;

const float range = 0.5f;
const float power = 0.05f;
const float max_magnitude = 0.3f;

vec4 explode(vec4 position, vec3 direction, vec3 normal, float magnitude)
{
    direction = direction * magnitude;
    // Normal = mat3(transpose(inverse(model))) * -normalize(normal + direction);
    // Normal = -normalize(normal + direction);
    vec4 newPosition = position + vec4(direction, 0.0);
    // vec4 focus = position + vec4(direction/2, 0.0);
    // Normal = normalize(hitPoint - focus.xyz);
    return newPosition;
}

float getDistance(vec3 point1, vec3 point2)
{
    return sqrt( pow(point1.x - point2.x, 2) + pow(point1.y - point2.y, 2) + pow(point1.z - point2.z, 2) );
}

void main()
{
	FragPos = model * vec4(position, 1.0);

	float distance = getDistance(hitPoint, FragPos.xyz);

	if (distance < range)
	{
		float magnitude = power/ distance;
		magnitude = min(magnitude, max_magnitude);
		FragPos = explode(FragPos, hitDirection, -normal, magnitude);
		FragPos = inverse(model) * FragPos;
		// FragPos = vec4(0);
		Normal = normalize(normal + hitDirection*magnitude);
	}
	else
	{
		FragPos = vec4(position, 1.0);
	    Normal = normal;
	}

    outValue = FragPos.xyz;
    outValue2 = Normal.xyz;
    // outValue = vec3(distance);
}