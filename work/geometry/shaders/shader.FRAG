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

// the transformed normal (in view coordinate) is set as an output variable, to be "passed" to the fragment shader
// this means that the normal values in each vertex will be interpolated on each fragment created during rasterization between two vertices
in vec3 Normal;

in vec2 TexCoords;

// in the fragment shader, we need to calculate also the reflection vector for each fragment
// to do this, we need to calculate in the vertex shader the view direction (in view coordinates) for each vertex, and to have it interpolated for each fragment by the rasterization stage
in vec3 viewPosition;

uniform vec3 viewPos;

uniform float materialShininess;

uniform sampler2D texture1;

// output shader variable
out vec4 colorFrag;


float getDistance(vec4 point1, vec4 point2)
{
	return sqrt( pow(point1.x - point2.x, 2) + pow(point1.y - point2.y, 2) + pow(point1.z - point2.z, 2) );
}


void main()
{
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - vec3(FragPos.xyz));
    vec3 result = texture(texture1, TexCoords).xyz;
    // vec3 result = vec3(0.1f);
    // phase 1: Directional lighting
    if (dirLight.on)
        result += CalcDirLight(dirLight, norm, viewDir);
    // phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        if(pointLights[i].on)
            result += CalcPointLight(pointLights[i], norm, vec3(FragPos.xyz), viewDir);    
    // phase 3: Spot light
    if (spotLight.on)
    	result += CalcSpotLight(spotLight, norm, vec3(FragPos.xyz), viewDir);    
    
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
	vec3 lightDir = normalize(light.position - vec3(FragPos.xyz));
    
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
    
    // if(theta > light.cutOff) // remember that we're working with angles as cosines instead of degrees so a '>' is used.
    // {    
    //     // diffuse 
    //     float diff = max(dot(norm, lightDir), 0.0);
    //     diffuse *= diff;  
        
    //     // specular
    //     vec3 viewDir = normalize(viewPos - vec3(FragPos.xyz));
    //     vec3 reflectDir = reflect(-lightDir, norm);  
    //     float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
    //     specular *= spec;  
        
    //     // attenuation
    //     float distance    = length(light.position - vec3(FragPos.xyz));
    //     float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    //     // ambient  *= attenuation; // remove attenuation from ambient, as otherwise at large distances the light would be darker inside than outside the spotlight due the ambient term in the else branche
    //     diffuse   *= attenuation;
    //     specular *= attenuation;   
            
    //     result = ambient + diffuse + specular;
    // }

    if (theta > light.outerCutOff)
    {
    	// diffuse 
        float diff = max(dot(norm, lightDir), 0.0);
        diffuse *= diff;  
        
        // specular
        vec3 viewDir = normalize(viewPos - vec3(FragPos.xyz));
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
        specular *= spec;  
        
        // attenuation
        float distance    = length(light.position - vec3(FragPos.xyz));
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

       	// spotlight (soft edges)
	    diffuse  *= intensity;
	    specular *= intensity;

        // ambient  *= attenuation; // remove attenuation from ambient, as otherwise at large distances the light would be darker inside than outside the spotlight due the ambient term in the else branche
        diffuse   *= attenuation;
        specular *= attenuation;   

	    result = ambient + diffuse + specular;
    }

    return result;



	// // ambient
 //    vec3 ambient = light.ambient; //texture(material.diffuse, TexCoords).rgb;
    
 //    // diffuse
 //    vec3 lightDir = normalize(light.position - fragPos);
 //    float diff = max(dot(norm, lightDir), 0.0);
 //    vec3 diffuse = light.diffuse * diff; //texture(material.diffuse, TexCoords).rgb;  
    
 //    // specular
 //    vec3 reflectDir = reflect(-lightDir, norm);  
 //    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
 //    vec3 specular = light.specular * spec; //texture(material.specular, TexCoords).rgb;  
    
 //    // spotlight (soft edges)
 //    float theta = dot(lightDir, normalize(-light.direction)); 
 //    float epsilon = (light.cutOff - light.outerCutOff);
 //    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
 //    diffuse  *= intensity;
 //    specular *= intensity;
    
 //    // attenuation
 //    float distance    = length(light.position - fragPos);
 //    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
 //    ambient  *= attenuation; 
 //    diffuse   *= attenuation;
 //    specular *= attenuation;   
        
 //    return (ambient + diffuse + specular);
}
