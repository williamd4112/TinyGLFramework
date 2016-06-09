#version 410 core

uniform sampler2D tex0; //sceneColor
uniform sampler2D tex1;	//glowMaskColor

layout (location = 0) out vec4 fragColor;

in VS_OUT 
{ 
	vec2 texcoord; 
} fs_in;


vec4 BloomEffect(){
	
	vec4 sum = vec4(0);
	vec2 texcoord = fs_in.texcoord.xy;

	int half_size = 12; 

   for(int i= - half_size ;i <  half_size; i++){
        for (int j = - half_size; j <  half_size; j++){
            sum += texture2D(tex1 , texcoord + vec2(j, i) * 0.004) * 0.25;
        }
   }

	if (texture2D( tex1 , texcoord).r < 0.3){
		return sum * sum * 0.012 + texture2D( tex1 , texcoord);
    }
    else{
        if (texture2D( tex1 , texcoord).r < 0.5){
           return sum * sum * 0.009 + texture2D( tex1 , texcoord);
        }
        else{
			return sum * sum * 0.0075 + texture2D( tex1 , texcoord);
        }
    }

}

void main(void) 
{ 

	//color = texture(tex0, fs_in.texcoord) + texture(tex1, fs_in.texcoord);
	//fragColor = texture(tex1, fs_in.texcoord);

	fragColor = BloomEffect() +  texture(tex0 , fs_in.texcoord);
	//fragColor = BloomEffect();
}