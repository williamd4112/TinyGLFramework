#version 410 core
out vec4 color;
in vec2 TexCoords;

uniform sampler2D depthMap;

void main()
{             
    float depthValue = texture(depthMap, TexCoords).r;
    color = vec4(vec3(1.0 - depthValue), 1.0);
}  