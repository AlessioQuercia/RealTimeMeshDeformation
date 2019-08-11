#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec4 worldPos;
} gs_in[];

uniform vec3 impactPoints[1000];

out vec4 vPos;

out vec4 vertices[3];

// light incidence direction (in view coordinates)	
in vec3 lightDir[3];
// the transformed normal (in view coordinate) is set as an output variable, to be "passed" to the fragment shader
// this means that the normal values in each vertex will be interpolated on each fragment created during rasterization between two vertices
in vec3 vNormal[3];

// in the fragment shader, we need to calculate also the reflection vector for each fragment
// to do this, we need to calculate in the vertex shader the view direction (in view coordinates) for each vertex, and to have it interpolated for each fragment by the rasterization stage
in vec3 vViewPosition[3];

out vec3 lightDire[3];
out vec3 vNorm[3];
out vec3 vViewPos[3];

vec4 explode(vec4 position, vec3 normal, float mag)
{
    float magnitude = mag;
    vec3 direction = normal * magnitude; 
    return position + vec4(direction, 0.0);
}

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

vec3 GetOppositeNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return -normalize(cross(a, b));
}

float getDistance(vec3 point1, vec3 point2)
{
	return sqrt( pow(point1.x - point2.x, 2) + pow(point1.y - point2.y, 2) + pow(point1.z - point2.z, 2) );
}

void main() 
{    
	for (int i = 0; i<3; i++)
	{
		vertices[i] = gl_in[i].gl_Position;
		lightDire[i] = lightDir[i];
		vNorm[i] = vNormal[i];
		vViewPos[i] = vViewPosition[i];
	}

    vec3 normal = GetOppositeNormal();

    bool done1 = false;
    bool done2 = false;
    bool done3 = false;

    for (int i = 0; i<1000; i++)
    {
    	if (getDistance(impactPoints[i], vec3(gs_in[0].worldPos.x, gs_in[0].worldPos.y, gs_in[0].worldPos.z)) < 0.5)
    	{
			gl_Position = explode(gl_in[0].gl_Position, normal, 0.1f);
		    EmitVertex();
		    done1 = true;
		    vPos = gl_Position;
		    break;
    	}
    }

    if (!done1)
    {
    	gl_Position = gl_in[0].gl_Position;
	    EmitVertex();
    }

    for (int i = 0; i<1000; i++)
    {
    	if (getDistance(impactPoints[i], vec3(gs_in[1].worldPos.x, gs_in[1].worldPos.y, gs_in[1].worldPos.z)) < 0.5)
    	{
			gl_Position = explode(gl_in[1].gl_Position, normal, 0.1f);
		    EmitVertex();
		    vPos = gl_Position;
		    done2 = true;
		    break;
    	}
    }

    if (!done2)
    {
    	gl_Position = gl_in[1].gl_Position;
	    EmitVertex();
    }

    for (int i = 0; i<1000; i++)
    {
    	if (getDistance(impactPoints[i], vec3(gs_in[2].worldPos.x, gs_in[2].worldPos.y, gs_in[2].worldPos.z)) < 0.5)
    	{
			gl_Position = explode(gl_in[2].gl_Position, normal, 0.1f);
		    EmitVertex();
		    vPos = gl_Position;
		    done3 = true;
		    break;
    	}
    }

    if (!done3)
    {
    	gl_Position = gl_in[2].gl_Position;
	    EmitVertex();
    }

    EndPrimitive();
}