/*
23_shadowmap.vert: vertex shader for the creation of the shadow map

It applies to the vertex the projection and view transformations of the light (used as a camera)

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2018/2019
Master degree in Computer Science
Universita' degli Studi di Milano
*/


#version 330 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;

// projection and view matrix -> vertex coordinates will be expressed using the light position as origin
uniform mat4 lightSpaceMatrix;

// model matrix
uniform mat4 modelMatrix;

void main()
{
	  // we apply the transformation to express the vertex from the point of view of the light
    gl_Position = lightSpaceMatrix * modelMatrix * vec4(position, 1.0f);
}
