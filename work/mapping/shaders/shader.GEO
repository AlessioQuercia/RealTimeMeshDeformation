#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 vviewPosition[3];
in vec4 vFragPos[3];

out vec3 Normal;
out vec4 FragPos;
out vec3 viewPosition;

vec3 GetNormal(int ind)
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[ind].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[ind].gl_Position);
    return normalize(cross(a, b));
}

void main() 
{    
    Normal = GetNormal(0);
    FragPos = vFragPos[0];
    viewPosition = vviewPosition[0];
	gl_Position = gl_in[0].gl_Position; 
    EmitVertex();

    Normal = GetNormal(1);
    FragPos = vFragPos[1];
    viewPosition = vviewPosition[1];
    gl_Position = gl_in[1].gl_Position; 
    EmitVertex();

    Normal = GetNormal(2);
    FragPos = vFragPos[2];
    viewPosition = vviewPosition[2];
    gl_Position = gl_in[2].gl_Position; 
    EmitVertex();

    EndPrimitive();
}