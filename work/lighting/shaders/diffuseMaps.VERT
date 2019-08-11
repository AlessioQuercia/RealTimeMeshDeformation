#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;  
out vec3 Normal;
out vec2 TexCoords;
  
void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    // We cast the Model matrix to a 3x3 matrix to ensure it loses its translations properties (last column), then we transform it to World Coords
    // this transform is called Normal Matrix and is computed as the transpose of the inverse of the upper-left corner of the Model Matrix
    // INVERSE TRANSOFRMATIONS ARE INEFFICIENT TO COMPUTE ON SHADERS: COMPUTE THEM IN THE CPU PART AND SEND THEM AS UNIFORMS!!
    Normal = Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
}