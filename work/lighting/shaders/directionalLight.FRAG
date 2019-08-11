#version 330 core

struct Material 
{
    sampler2D diffuse;
    sampler2D specular;
    //sampler2D emission;
    float     shininess;
}; 

struct Light {
    // vec3 position; // No longer necessery when using directional lights.
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
  
uniform Material material;

uniform Light light;

uniform vec3 viewPos;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

void main()
{
 
	// Phong Lighting (Ambient + Diffuse + Specular)
    // Ambient Lighting
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
  	
    // Diffuse Lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);   
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    
    // Specular Lighting
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));  

    // Emission
    //vec3 emission = texture(material.emission, TexCoords).rgb;
        
    //vec3 result = ambient + diffuse + specular + emission;
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0); 
} 