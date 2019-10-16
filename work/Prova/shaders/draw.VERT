#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;
out vec4 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform int change;

void main()
{
    TexCoords = aTexCoords;
    FragPos = vec4(aPos, 1.0);
    // if (change==1)
    // 	FragPos = vec4(aPos.x, aPos.y, aPos.z-1, 1.0);
    gl_Position = model*vec4(aPos, 1.0);
}
