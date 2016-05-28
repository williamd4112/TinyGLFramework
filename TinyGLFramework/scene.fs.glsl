#version 410

layout(location = 0) out vec4 fragColor;

in vec3 vv3normal;
in vec3 vv3tangent;
in vec2 vv2texcoord;
in mat3 matTBN;

uniform sampler2D sampler;   
uniform sampler2D normalmap;

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

uniform vec3 E;
uniform int Mode;
uniform mat4 M, V;

vec4 calc_dirLight(int light_id)
{
	vec3 vertex_pos = vv3pos;
	vec3 normal = normalize(texture(normalmap, vv2texcoord).rgb * 2.0 - vec3(1.0));

	float shininess = Material.shininess;
	
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

	return vec4(ambient + diffuse + specular, 1.0);
}


void main() 
{
	vec4 color = texture2D(sampler, vv2texcoord.st).rgba;
	if(color.a < 0.5)
		discard;
	else
	{
		vec3 intensity = vec3(0, 0, 0);
		for(int i = 0; i < 2; i++)
		{
			vec3 tmp = calc_dirLight(i).rgb;
			
			if(bool(LightSource[i].type & 0x1))
			{
				float lightdist = length(vec3(V * LightSource[i].position) - vv3pos);
				float att = 1.0 / (LightSource[i].constantAttenuation +
						LightSource[i].linearAttenuation * lightdist +
						LightSource[i].quadraticAttenuation * lightdist * lightdist);
				tmp *= att;
			}
			intensity += tmp;
		}

		fragColor = vec4(color.rgb * intensity , color.a);
	}
}