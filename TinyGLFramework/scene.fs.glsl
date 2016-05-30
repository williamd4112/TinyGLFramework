#version 410

layout(location = 0) out vec4 fragColor;

in vec3 vv3normal;
in vec3 vv3tangent;
in vec2 vv2texcoord;
in mat3 matTBN;
in vec3 worldPos;
in vec4 vv4posLightSpace;

uniform sampler2D sampler;   
uniform sampler2D normalmap;
uniform sampler2D depthmap;
uniform samplerCube depthcube;
uniform sampler2D specularmap;

in vec3 vv3pos;

struct LightSourceParameters {
	int type;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position;
	vec4 halfVector;
	vec4 spotDirection;
	float spotExponent;
	float spotCutoff; // (range: [0.0,90.0], 180.0)
	float spotCosCutoff; // (range: [1.0,0.0],-1.0)
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
};

struct MaterialParameters {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

uniform MaterialParameters Material;
uniform LightSourceParameters LightSource[3];

uniform vec3 lightPos;
uniform float far_plane;
uniform vec3 E;
uniform int Mode;
uniform int enable_specularMap = 0;
uniform mat4 M, V;

uniform int fog_type = 2;
const vec4 fogColor = vec4(0.5, 0.5, 0.5,1.0);
float fogFactor = 0;
float fogDensity = 0.00025;
float fog_start = 1;
float fog_end = 1000.0f;

vec4 calc_dirLight(int light_id, float shadow)
{
	vec3 vertex_pos = vv3pos;
	vec3 normal = normalize(texture(normalmap, vv2texcoord).rgb * 2.0 - vec3(1.0));

	float shininess = (enable_specularMap == 1) ? texture(specularmap, vv2texcoord).r * 128.0 : Material.shininess;
	shininess = clamp(shininess, 2.0, 128.0);
	vec3 diffuse_color = vec3(LightSource[light_id].diffuse); 
	vec3 ambient_color = vec3(LightSource[light_id].ambient); 
	vec3 specular_color = vec3(LightSource[light_id].specular); 
	vec3 position = vec3(V * LightSource[light_id].position); 
	
	vec3 lightdir = normalize(normalize(position) - normalize(vertex_pos));
	lightdir = normalize(matTBN * lightdir);

	float lambert_coff = max(dot(normal, -lightdir), 0.0); 

	vec3 reflection = normalize(reflect(-lightdir, normal));
	vec3 viewdir = normalize(matTBN * -vertex_pos);

	vec3 ambient = ambient_color * Material.ambient.rgb;
	vec3 diffuse = lambert_coff * diffuse_color * Material.diffuse.rgb;
	vec3 specular = vec3(0, 0, 0);
	float RdotE = max(dot(reflection, viewdir), 0.0);
	specular = pow(RdotE, shininess) * specular_color * Material.specular.rgb;

	return vec4(ambient + 1.5 * (1.0 - shadow) * (diffuse + specular), 1.0);
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = vv4posLightSpace.xyz / vv4posLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	 
	float closestDepth = texture(depthmap, projCoords.xy).r; 
	float currentDepth = projCoords.z;  
	return currentDepth > closestDepth  ? 1.0 : 0.0;  
}

float OminiShadowCalculation(vec3 fragPos)
{
    // Get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;

    // Use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(depthcube, fragToLight).r;

    // It is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= far_plane;

    // Now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);

    // Now test for shadows
    float bias = 0.01; 
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}  

vec4 atmospheric(vec4 color)
{
	float dist = length(vv3pos);
	float be = (E.y - vv3pos.y) * 0.8;

	//0.004 is just a factor; change it if you want
	float bi = (E.y - vv3pos.y) * 0.00007;
	float ext = exp(-dist * be);
	float insc = exp(-dist * bi);

	return color * ext + fogColor * (1-insc);
}

void main() 
{
	vec4 color = texture2D(sampler, vv2texcoord.st).rgba;

	if(color.a < 0.5)
		discard;
	else
	{
		vec3 intensity = vec3(0, 0, 0);
		for(int i = 0; i < 1; i++)
		{
			// Calculate shadow
			float shadow = 0.0;
			if(bool(LightSource[i].type & 0x1))
			{
				shadow = OminiShadowCalculation(worldPos);  
			}
			else if(LightSource[i].type == 0)
			{
				shadow = ShadowCalculation(vv4posLightSpace);  
			}

			// Calculate light
			vec3 tmp = calc_dirLight(i, shadow).rgb;
			if(bool(LightSource[i].type & 0x1))
			{
				float lightdist = length(vec3(V * LightSource[i].position) - vv3pos);
				float att = 1.0 / (LightSource[i].constantAttenuation +
						LightSource[i].linearAttenuation * lightdist +
						LightSource[i].quadraticAttenuation * lightdist * lightdist);
				tmp *= att * 1.2;
			}
			else
			{
				tmp *= 1.1;
			}
			intensity = intensity + tmp;
		}

		
		float dist = length(vv3pos);
		switch(fog_type)
		{
			case 0: //Linear
				fogFactor = (fog_end - dist) / (fog_end - fog_start);
				break;
			case 1: //Exp
				fogFactor = 1.0 /exp(dist * fogDensity);
				break;
			case 2: //Exp sqare
				fogFactor = 1.0 /exp( (dist * fogDensity)* (dist * fogDensity));
				break;
			default:
				break;
		}
		fogFactor = 1.0 / exp(dist * fogDensity);	
		fogFactor = clamp( fogFactor, 0.0, 1.0 );

		fragColor = vec4(color.rgb * intensity , color.a);
	    //fragColor = mix(fogColor, fragColor , fogFactor);
		//fragColor = atmospheric(fragColor);
	}
}