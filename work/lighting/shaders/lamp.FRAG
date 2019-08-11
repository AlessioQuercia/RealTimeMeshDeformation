#version 330 core

uniform bool on;

out vec4 FragColor;

void main()
{
	vec4 color;
	if (on)
		color = vec4(1.0);
	else
		color = vec4(0.0);

    FragColor = color;
}