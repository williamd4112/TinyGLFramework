#version 410 core

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;

uniform float midRatio;
uniform int finalMode;

layout (location = 0) out vec4 color;

in VS_OUT 
{ 
	vec2 texcoord; 
} fs_in;

vec4 blur(sampler2D tex)
{
	vec4 color = vec4(0); 
	
	int n = 0; 
	int half_size = 16; 
	
	//filter_half_size 
	for ( int i = - half_size; i <= half_size ; ++i ) 
	{        
		for ( int j = - half_size; j <= half_size ; ++j ) 
		{ 
			vec4 c = texture(tex,fs_in.texcoord + vec2(i, j) / 1024); 
			color += c; 
			n++;
		} 
	} 
	color /= n;
	
	return color;
}

void main(void) 
{ 
	float d = 0.001;
	color = texture(tex0, fs_in.texcoord);
	color = vec4(vec3(1.0, 1.0, 1.0) - color.rgb, 1.0);
}