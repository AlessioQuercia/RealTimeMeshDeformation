#version 330 core


struct DirLight {
    vec3 direction;
    bool on;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {    
    vec3 position;
    bool on;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

struct SpotLight {    
    vec3  position;
    vec3  direction;
    float cutOff;
    float outerCutOff;
    bool on;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
}; 

// Functions
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir); 
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir); 

#define NR_POINT_LIGHTS 4  
uniform PointLight pointLights[NR_POINT_LIGHTS]; 

uniform DirLight dirLight;

uniform SpotLight spotLight;

in vec4 FragPos;
in vec2 TexCoords;

// in the fragment shader, we need to calculate also the reflection vector for each fragment
// to do this, we need to calculate in the vertex shader the view direction (in view coordinates) for each vertex, and to have it interpolated for each fragment by the rasterization stage
in vec3 viewPosition;

in vec3 Normal;

uniform vec3 viewPos;

uniform float materialShininess;

// output shader variable
out vec4 colorFrag;

uniform sampler2D texture1;

float getDistance(vec4 point1, vec4 point2)
{
	return sqrt( pow(point1.x - point2.x, 2) + pow(point1.y - point2.y, 2) + pow(point1.z - point2.z, 2) );
}

void main()
{           
    // obtain normal from normal map in range [0,1]
    // vec3 norm = texture(normalMap, TexCoords).rgb;
    // transform normal vector to range [-1,1]
    vec3 norm = normalize(Normal);  

    // properties
    vec3 viewDir = normalize(viewPos - FragPos.xyz);
    vec3 result = texture(texture1, TexCoords).xyz; //vec3(0.1f);
    // vec3 result = vec3(0.1f, 0.1f, 0.1f);                           
    // phase 1: Directional lighting
    if (dirLight.on)
        result += CalcDirLight(dirLight, norm, viewDir);
    // phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        if(pointLights[i].on)
            result += CalcPointLight(pointLights[i], norm, FragPos.xyz, viewDir);    
    // phase 3: Spot light
    if (spotLight.on)
    	result += CalcSpotLight(spotLight, norm, FragPos.xyz, viewDir);    
    
    colorFrag = vec4(result, 1.0);
}



vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
    // combine results
    vec3 ambient  = light.ambient; //vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.diffuse  * diff; //vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec; //vec3(texture(material.specular, TexCoords));
    return (ambient + diffuse + specular);
} 

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = light.ambient; //vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.diffuse  * diff; //vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec; //vec3(texture(material.specular, TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
    
    // check if lighting is inside the spotlight cone
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon   = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0); 

    vec3 result = vec3(0.0f, 0.0f, 0.0f);

    // ambient
    vec3 ambient = light.ambient;

    // diffuse
    vec3 diffuse = light.diffuse;

    // specular
    vec3 specular = light.specular;

    if (theta > light.outerCutOff)
    {
    	// diffuse 
        float diff = max(dot(norm, lightDir), 0.0);
        diffuse *= diff;  
        
        // specular
        vec3 viewDir = normalize(viewPos - fragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
        specular *= spec;  
        
        // attenuation
        float distance    = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

       	// spotlight (soft edges)
	    diffuse  *= intensity;
	    specular *= intensity;

        // ambient  *= attenuation; // remove attenuation from ambient, as otherwise at large distances the light would be darker inside than outside the spotlight due the ambient term in the else branche
        diffuse  *= attenuation;
        specular *= attenuation;   

	    result = ambient + diffuse + specular;
    }

    return result;
}
