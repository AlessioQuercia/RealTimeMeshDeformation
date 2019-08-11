/*
14_ggx.frag: Fragment shader for the GGX illumination model

N.B.)  "13_phong.vert" must be used as vertex shader

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2018/2019
Master degree in Computer Science
Universita' degli Studi di Milano

*/


#version 330 core


uniform float materialShininess;


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

in vec4 vertex;

in vec4 FragPos;

#define NR_POINT_LIGHTS 4  
uniform PointLight pointLights[NR_POINT_LIGHTS]; 

uniform DirLight dirLight;

uniform SpotLight spotLight;

// uniform vec3 viewPos;

const float PI = 3.14159265359;

// in vec3 lightDire[3];
// in vec3 vNorm[3];
// in vec3 vViewPos[3];

// light incidence direction (in view coordinates)	
in vec3 lightDir;
in vec3 lightDir1;
// the transformed normal (in view coordinate) is set as an output variable, to be "passed" to the fragment shader
// this means that the normal values in each vertex will be interpolated on each fragment created during rasterization between two vertices
in vec3 vNormal;

// in the fragment shader, we need to calculate also the reflection vector for each fragment
// to do this, we need to calculate in the vertex shader the view direction (in view coordinates) for each vertex, and to have it interpolated for each fragment by the rasterization stage
in vec3 vViewPosition;

uniform vec3 viewPos;

// output shader variable
out vec4 colorFrag;

// diffusive component (passed from the application)
uniform vec3 diffuseColor;
uniform float alpha; // rugosity - 0 : smooth, 1: rough
uniform float F0; // fresnel reflectance at normal incidence
uniform float Kd; // weight of diffuse reflection

float G1(float angle, float alpha)
{
    // in case of Image Based Lighting, the k factor is different:
    // usually it is set as k=(alpha*alpha)/2
    float r = (alpha + 1.0);
    float k = (r*r) / 8.0;

    float num   = angle;
    float denom = angle * (1.0 - k) + k;

    return num / denom;
}

float getDistance(vec4 point1, vec4 point2)
{
	return sqrt( pow(point1.x - point2.x, 2) + pow(point1.y - point2.y, 2) + pow(point1.z - point2.z, 2) );
}

void main2()
{
	// normalization of the per-fragment normal
    vec3 N = normalize(vNormal);
    // normalization of the per-fragment light incidence direction
    vec3 L = normalize(lightDir.xyz);
    vec3 L1 = normalize(lightDir1.xyz);

    // cosine angle between direction of light and normal
    float NdotL = max(dot(N, L), 0.0);
    float NdotL1 = max(dot(N, L1), 0.0);

    vec3 color = vec3(0.2, 0.2, 0.2);

    // diffusive (Lambert) reflection component
    vec3 lambert = (Kd*color)/PI;

    // we initialize the specular component
    vec3 specular = vec3(0.0);
    vec3 specular1 = vec3(0.0);

    // if the cosine of the angle between direction of light and normal is positive, then I can calculate the specular component
    if(NdotL > 0.0)
    {
        // the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
        vec3 V = normalize( viewPos );

        // half vector
        vec3 H = normalize(L + V);
        vec3 H1 = normalize(L1 + V);

        // we implement the components seen in the slides for a PBR BRDF
        // we calculate the cosines and parameters to be used in the different components
        float NdotH = max(dot(N, H), 0.0);
        float NdotH1 = max(dot(N, H1), 0.0);
        float NdotV = max(dot(N, V), 0.0);
        float VdotH = max(dot(V, H), 0.0);
        float VdotH1 = max(dot(V, H1), 0.0);
        float alpha_Squared = alpha * alpha;
        float NdotH_Squared = NdotH * NdotH;
        float NdotH1_Squared = NdotH1 * NdotH1;

        // Geometric factor G2
        float G2 = G1(NdotV, alpha)*G1(NdotL, alpha);
        float G21 = G1(NdotV, alpha)*G1(NdotL1, alpha);

        // Rugosity D
        // GGX Distribution
        float D = alpha_Squared;
        float D1 = alpha_Squared;
        float denom = (NdotH_Squared*(alpha_Squared-1.0)+1.0);
        float denom1 = (NdotH1_Squared*(alpha_Squared-1.0)+1.0);
        D /= PI*denom*denom;
        D1 /= PI*denom1*denom1;

        // Fresnel reflectance F (approx Schlick)
        vec3 F = vec3(pow(1.0 - VdotH, 5.0));
        vec3 F1 = vec3(pow(1.0 - VdotH1, 5.0));
        F *= (1.0 - F0);
        F1 *= (1.0 - F0);
        F += F0;
        F1 += F0;

        // we put everything together for the specular component
        specular = (F * G2 * D) / (4.0 * NdotV * NdotL);
        specular1 = (F1 * G21 * D1) / (4.0 * NdotV * NdotL1);
    }

    // the rendering equation is:
    //integral of: BRDF * Li * (cosine angle between N and L)
    // BRDF in our case is: the sum of Lambert and GGX
    // Li is considered as equal to 1: light is white, and we have not applied attenuation. With colored lights, and with attenuation, the code must be modified and the Li factor must be multiplied to finalColor
    vec3 finalColor = (lambert + specular)*NdotL;

    colorFrag = vec4(finalColor, 1.0);


	// bool red = false;

 //    for (int i = 0; i<3; i++)
 //    {
	// 	if (getDistance(FragPos, vertices[i]) < 0.5f)
	// 	{
	// 		red = true;
	// 		break;
	// 	}
 //    }

 //    if (red)
 //    {
 //    	colorFrag = vec4(1.0, 0.0, 0.0, 1.0);
 //    }
 //    else
 //    {
 //    	colorFrag = vec4(finalColor, 1.0);
 //    }
}

void main()
{
    // properties
    vec3 norm = normalize(vNormal);
    vec3 viewDir = normalize(viewPos - vec3(FragPos.xyz));
    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    // phase 1: Directional lighting
    if (dirLight.on)
        result = CalcDirLight(dirLight, norm, viewDir);
    // phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        if(pointLights[i].on)
            result += CalcPointLight(pointLights[i], norm, vec3(FragPos.xyz), viewDir);    
    // phase 3: Spot light
    if (spotLight.on)
    	result += CalcSpotLight(spotLight, norm, vec3(FragPos.xyz), viewDir);    
    
    colorFrag = vec4(result, 1.0);

 //    bool red = false;

	// if (getDistance(FragPos, vertex) != 0)
	// {
	// 	red = true;
	// }

 //    if (red)
 //    {
 //    	colorFrag = vec4(1.0, result.y, result.z, 1.0);
 //    }
 //    else
 //    {
 //    	colorFrag = vec4(result, 1.0);
 //    }
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
	// ambient
    vec3 ambient = light.ambient; //texture(material.diffuse, TexCoords).rgb;
    
    // diffuse 
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff; //texture(material.diffuse, TexCoords).rgb;  
    
    // specular
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
    vec3 specular = light.specular * spec; //texture(material.specular, TexCoords).rgb;  
    
    // // spotlight (soft edges)
    // float theta = dot(lightDir, normalize(-light.direction)); 
    // float epsilon = (light.cutOff - light.outerCutOff);
    // float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // diffuse  *= intensity;
    // specular *= intensity;
    
    // // attenuation
    // float distance    = length(light.position - fragPos);
    // float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // ambient  *= attenuation; 
    // diffuse   *= attenuation;
    // specular *= attenuation;   
        
    return (ambient + diffuse + specular);
}
