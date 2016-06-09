#version 410 core

uniform sampler2D tex0; //sceneColor
uniform sampler2D tex1;	//waterNormalMaskColor
uniform sampler2D tex2; //glowMaskColor
uniform sampler2D tex3; //reflectColor

uniform sampler2D waterDuDv;

uniform float time;

layout (location = 0) out vec4 sceneColor;
layout (location = 1) out vec4 glowMaskColor;

in VS_OUT 
{ 
	vec2 texcoord; 
} fs_in;


int isWater( vec2 texcoord ){
	
	float black = 0.0000001;

	vec4 mask = texture(tex1, texcoord);

	if( mask.r < black &&  mask.g < black  && mask.b < black)
		return 1;
	else return 0;
}


void main(void) 
{ 
	vec2 texcoord = fs_in.texcoord.xy;

	vec2 d1 = texture( waterDuDv ,  vec2 ( texcoord.x + time , texcoord.y )  ).rg * 2.0 - 1.0;
	vec2 d2 = texture( waterDuDv ,  vec2 ( - texcoord.x + time , texcoord.y + time )  ).rg * 2.0 - 1.0;
	vec2 d3 = d1 + d2;

	
	if( isWater(texcoord) == 1){

		sceneColor = texture(tex0, fs_in.texcoord);
	
	}
	else {

		vec2 R = vec2(  texcoord.x , 1.0 - texcoord.y  ) + d3 / 100;
		vec2 S = texcoord + d3 / 100;
		vec2 N = texcoord;
		sceneColor = mix( texture( tex3, R ) , texture( tex0, S ) * texture( tex1,  texcoord + d3 / 200.0)  , 0.5);	
	}

	if( isWater(texcoord) == 0)
	{
		sceneColor += vec4 (-0.3, -0.1 , 0.0 , 1.0 );
		sceneColor *= 1.5;
	}

	//if( isWater(texcoord) == 0)sceneColor += texture( tex1, texcoord );

	glowMaskColor = texture(tex2, fs_in.texcoord);

}