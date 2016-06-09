#version 410

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec3 iv3normal;
layout(location = 2) in vec2 iv2texcoord;

uniform mat4 mvp;
uniform mat4 M, V, P;

uniform float water_H;

out vec4 vv4color;
out vec3 vv3normal;
out vec3 vv3pos;
out vec2 vv2texcoord;

void main()
{

	mat4 N = transpose(inverse(V * M));

	gl_Position = mvp * vec4(iv3vertex, 1.0);
	vv2texcoord = iv2texcoord;

	vec3 normal = mat3(N) * iv3normal;
	vv3normal = normalize(normal);

	vec4 vertPos4 = (V * M) * vec4(iv3vertex, 1);
	vv3pos = vec3(vertPos4) / vertPos4.w;

	vec4 worldPos = M * vec4 ( iv3vertex , 1.0 );
	vec4 plane = vec4( 0, 1, 0, water_H);

	gl_ClipDistance[0] = dot(worldPos , plane );

}
