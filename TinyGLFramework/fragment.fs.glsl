#version 410

layout(location = 0) out vec4 fragColor;

in vec3 vv3normal;
in vec2 vv2texcoord;

uniform sampler2D sampler;   

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

vec4 calc_dirLight()
{
	vec4 color = texture2D(sampler, vv2texcoord.st);
	if(color.a < 0.5)
		discard;

	vec3 vertex_pos = vv3pos;
	vec3 normal = normalize(vv3normal);

	float light_intensity = 1.0;
	float shininess = Material.shininess;
	
	vec3 diffuse_color = vec3(LightSource[0].diffuse); 
	vec3 ambient_color = vec3(LightSource[0].ambient); 
	vec3 specular_color = vec3(LightSource[0].specular);
	vec3 position = vec3(V * LightSource[0].position);
	
	vec3 lightdir = normalize(position); 

	float lambert_coff = max(dot(normal, lightdir), 0.0); 

	vec3 reflection = normalize(reflect(-lightdir, normal)); 
	vec3 viewdir = normalize(E - vertex_pos); 

	vec3 ambient = ambient_color * vec3(Material.ambient) * vec3(color.rgb);
	vec3 diffuse = lambert_coff * diffuse_color * vec3(Material.diffuse) * color.rgb;
	vec3 specular = vec3(0, 0, 0);

	if (lambert_coff > 0.0) {

		float RdotE = max(dot(reflection, viewdir), 0.0);
		specular =  pow(RdotE, shininess) * specular_color * vec3(Material.specular) * color.rgb;
	}

	return vec4(ambient + diffuse + specular, color.a);
}

vec4 calc_pointLight()
{
	vec4 color = texture2D(sampler, vv2texcoord.st);
	if(color.a < 0.5)
		discard;

	vec3 vertex_pos = vv3pos;
	vec3 normal = normalize(vv3normal);

	float light_intensity = 1.0;
	float shininess = Material.shininess;
	
	vec3 diffuse_color = vec3(LightSource[1].diffuse); 
	vec3 ambient_color = vec3(LightSource[1].ambient); 
	vec3 specular_color = vec3(LightSource[1].specular); 
	vec3 position = vec3(LightSource[1].position); 
	
	vec3 lightdir = vertex_pos - vec3(V * vec4(position, 1.0));
	float lightdist = length(lightdir);
	lightdir = normalize(-lightdir);

	float lambert_coff = max(dot(normal, lightdir), 0.0); 

	vec3 reflection = normalize(reflect(-lightdir, normal));
	vec3 viewdir = normalize(E - vertex_pos); 

	float att = 1.0 / (LightSource[1].constantAttenuation +
                LightSource[1].linearAttenuation * lightdist +
                LightSource[1].quadraticAttenuation * lightdist * lightdist);

	vec3 ambient = light_intensity * ambient_color;
	vec3 diffuse = light_intensity * lambert_coff * diffuse_color * vec3(texture2D(sampler, vv2texcoord.st));
	vec3 specular = vec3(0, 0, 0);

	if (lambert_coff > 0.0) {

		float RdotE = max(dot(reflection, viewdir), 0.0);
		specular = light_intensity * pow(RdotE, shininess) * specular_color;
	}

	return att * vec4(ambient + diffuse + specular, color.a);
}

void main() {
	//gl_FragColor = vec4(1.0,0.0,0.0,1.0);

	vec4 color = texture2D(sampler, vv2texcoord.st);
	if(color.a < 0.5)
		discard;
	fragColor = color;
}