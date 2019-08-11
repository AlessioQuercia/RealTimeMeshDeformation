/*
13_phong.vert: Vertex shader for the Phong and Blinn-Phong illumination model

N. B.) the shader treats a simplified situation, with a single point light.
For more point lights, a for cycle is needed to sum the contribution of each light
For different kind of lights, the computation must be changed (for example, a directional light is defined by the direction of incident light, so the lightDir is passed as uniform and not calculated in the shader like in this case with a point light).

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2018/2019
Master degree in Computer Science
Universita' degli Studi di Milano

*/

#version 330 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// vertex normal in world coordinate
layout (location = 1) in vec3 normal;

// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;

// normals transformation matrix (= transpose of the inverse of the model-view matrix)
uniform mat3 normalMatrix;

// the position of the point light is passed as uniform
// N. B.) with more lights, and of different kinds, the shader code must be modified with a for cycle, with different treatment of the source lights parameters (directions, position, cutoff angle for spot lights, etc)
uniform vec3 pointLightPosition;
uniform vec3 pointLightPosition1;

uniform vec3 impactPoints[600];
uniform vec3 hittingDirections[600];

// light incidence direction (in view coordinates)	
out vec3 lightDir;
out vec3 lightDir1;
// the transformed normal (in view coordinate) is set as an output variable, to be "passed" to the fragment shader
// this means that the normal values in each vertex will be interpolated on each fragment created during rasterization between two vertices
out vec3 vNormal;

// in the fragment shader, we need to calculate also the reflection vector for each fragment
// to do this, we need to calculate in the vertex shader the view direction (in view coordinates) for each vertex, and to have it interpolated for each fragment by the rasterization stage
out vec3 vViewPosition;

out vec4 FragPos;

out vec4 vertex;

const float range = 0.3f;
const float power = 0.01f;

vec4 explode(vec4 position, vec3 direction, vec3 normal, float mag)
{
    // float magnitude = mag;
    // vec3 direction = normal * magnitude; 
    // return position + vec4(direction, 0.0);
    // return vec4(position.x, position.y, position.z-0.1, position.w);
    float magnitude = mag;
    // vec3 direction = normal * magnitude; 
    direction = direction * magnitude;
    return position + vec4(direction, 0.0);
}

float getDistance(vec3 point1, vec3 point2)
{
	return sqrt( pow(point1.x - point2.x, 2) + pow(point1.y - point2.y, 2) + pow(point1.z - point2.z, 2) );
}


void main()
{
	FragPos = modelMatrix * vec4(position, 1.0);

	vec4 worldPos = FragPos;

	for (int i = 0; i<1000; i++)
    {
    	float distance = getDistance(impactPoints[i], vec3(worldPos.x, worldPos.y, worldPos.z));

    	if (distance < range)
    	{
    		float magnitude = power/ distance;
			worldPos = explode(worldPos, hittingDirections[i], -normal, magnitude);
		    break;
    	}

   //  	if (distance < 0.4)
   //  	{
   //  		float magnitude = 1.0f * distance;
			// worldPos = explode(worldPos, hittingDirections[i], -normal, magnitude);
		 //    break;
   //  	}
    }

	// vertex position in ModelView coordinate (see the last line for the application of projection)
	// when I need to use coordinates in camera coordinates, I need to split the application of model and view transformations from the projection transformations
	vec4 mvPosition = viewMatrix * worldPos;

	// vec4 worldPosition = modelMatrix * vec4(position, 1.0);

	// for (int i = 0; i<1000; i++)
	// {
	// 	if (verticesToDeform[i].x == 0 && verticesToDeform[i].y == 0 && verticesToDeform[i].z == 0)
	// 		break;
	// 	if (worldPosition.x == verticesToDeform[i].x && worldPosition.y == verticesToDeform[i].y && worldPosition.z == verticesToDeform[i].z)
	// 	{
	// 	    float magnitude = 2.0;
	// 	    vec3 direction = normal * magnitude; 

	// 		// worldPosition = vec4(worldPosition.x, worldPosition.y, worldPosition.z + 0.5f, worldPosition.w);
	// 		mvPosition = viewMatrix * (worldPosition + vec4(-direction, 0.0));
	// 		// break;
	// 	}
	// }

	// if ( (worldPosition.x == deformVertex1.x && worldPosition.y == deformVertex1.y && worldPosition.z == deformVertex1.z) ||
	// 	 (worldPosition.x == deformVertex2.x && worldPosition.y == deformVertex2.y && worldPosition.z == deformVertex2.z) ||
	// 	 (worldPosition.x == deformVertex3.x && worldPosition.y == deformVertex3.y && worldPosition.z == deformVertex3.z) ||
	// 	 (worldPosition.x == deformVertex4.x && worldPosition.y == deformVertex4.y && worldPosition.z == deformVertex4.z) ||
	// 	 (worldPosition.x == deformVertex5.x && worldPosition.y == deformVertex5.y && worldPosition.z == deformVertex5.z) ||
	// 	 (worldPosition.x == deformVertex6.x && worldPosition.y == deformVertex6.y && worldPosition.z == deformVertex6.z) )
	// {
	// 	worldPosition = vec4(worldPosition.x + 2.0f, worldPosition.y + 2.0f, worldPosition.z + 2.0f, worldPosition.w);
	// 	mvPosition = viewMatrix * worldPosition;
	// }

	// view direction, negated to have vector from the vertex to the camera
	vViewPosition = -mvPosition.xyz;

	// transformations are applied to the normal
	vNormal = normalize( normalMatrix * normal );

	// light incidence direction (in view coordinate)
	vec4 lightPos = viewMatrix  * vec4(pointLightPosition, 1.0);
	lightDir = lightPos.xyz - mvPosition.xyz;

	vec4 lightPos1 = viewMatrix * vec4(pointLightPosition1, 1.0);
	lightDir1 = lightPos1.xyz - mvPosition.xyz;

	vertex = worldPos;

	// we apply the projection transformation
	gl_Position = projectionMatrix * mvPosition;

}