#version 410

layout(location = 0) out vec4 sceneColor;

in vec3 vv3normal;
in vec2 vv2texcoord;

uniform sampler2D sampler;

uniform int objectVariant;

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

void main() {

	vec4 color = texture2D(sampler, vv2texcoord.st);

	if(color.a < 0.5)
		discard;
	if( object_T != 1 )
		sceneColor = color;

}