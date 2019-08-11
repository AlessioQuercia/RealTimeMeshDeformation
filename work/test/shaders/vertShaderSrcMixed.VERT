#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform float xOffset;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 ourColor;
void main()
{
   //gl_Position = vec4(aPos.x + xOffset, aPos.y, aPos.z, 1.0); // add the xOffset to the x position of the vertex position

	gl_Position = projection * view * model * vec4(aPos.x + xOffset, aPos.y, aPos.z, 1.0);

   ourColor = aColor;
}