#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoords;
    vec3 pos;
} gs_in[];

out vec2 TexCoords; 

uniform float time;

vec4 explode(vec4 position, vec3 normal, float mag)
{
    float magnitude = mag;
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
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

void main() {    
    vec3 normal = GetOppositeNormal();

    // gl_Position = explode(gl_in[0].gl_Position, normal);
    // TexCoords = gs_in[0].texCoords;
    // EmitVertex();
    // gl_Position = explode(gl_in[1].gl_Position, normal);
    // TexCoords = gs_in[1].texCoords;
    // EmitVertex();

    if (gs_in[0].pos.x > 0.0 && gs_in[0].pos.x < 0.5 && gs_in[0].pos.y > 10.0 && gs_in[0].pos.y < 10.1)
    {
    	gl_Position = explode(gl_in[0].gl_Position, normal, 0.01);
	    TexCoords = gs_in[0].texCoords;
	    EmitVertex();
	}
	else
	{
	    gl_Position = gl_in[0].gl_Position;
	    TexCoords = gs_in[0].texCoords;
	    EmitVertex();
	}

	if (gs_in[1].pos.x > 0.0 && gs_in[1].pos.x < 0.5 && gs_in[1].pos.y > 10.0 && gs_in[1].pos.y < 10.1)
    {
    	gl_Position = explode(gl_in[1].gl_Position, normal, 0.01);
	    TexCoords = gs_in[1].texCoords;
	    EmitVertex();
    }
    else
    {
    	gl_Position = gl_in[1].gl_Position;
	    TexCoords = gs_in[1].texCoords;
	    EmitVertex();
    }

   	if (gs_in[2].pos.x > 0.0 && gs_in[2].pos.x < 0.5 && gs_in[2].pos.y > 10.0 && gs_in[2].pos.y < 10.1)
    {
    	gl_Position = explode(gl_in[2].gl_Position, normal, 0.01);
	    TexCoords = gs_in[2].texCoords;
	    EmitVertex();
    }
    else
    {
    	gl_Position = gl_in[2].gl_Position;
	    TexCoords = gs_in[2].texCoords;
	    EmitVertex();
    }

    EndPrimitive();
}