#version 410

layout (location = 0) in vec3 position;
out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 camera_position;

void main()
{
    gl_Position = projection * view * vec4(position.xyz + camera_position, 1.0);  
    TexCoords = position;
} 