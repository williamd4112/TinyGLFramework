#version 410 core

layout (location = 0) in vec2 position;                                                    
layout (location = 1) in vec2 texcoord;    

out vec2 TexCoords;

void main(void) 
{ 
	gl_Position = vec4(position, 1.0 ,1.0); 
	TexCoords = texcoord;
}
