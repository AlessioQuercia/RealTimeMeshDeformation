#version 330 core

in vec3 position;
in vec3 normal;
out vec3 outValue;

void main()
{
	if (position.x > 0.5 && position.y > 0.5 && position.z > 0.5)
    	outValue = vec3(position.x-0.5, position.y, position.z);
    else
    	outValue = position;
}