#version 410

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec3 iv3normal;
layout(location = 2) in vec2 iv2texcoord;
layout(location = 3) in vec3 iv3tangent;

uniform mat4 mvp;
uniform mat4 M, V, P;

out vec4 vv4color;
out vec3 vv3normal;
out vec3 vv3tangent;
out vec3 vv3pos;
out vec2 vv2texcoord;
out mat3 matTBN;

void main()
{
	mat4 N = transpose(inverse(V * M));

	gl_Position = mvp * vec4(iv3vertex, 1.0);
	vv2texcoord = iv2texcoord;

	vec3 normal = mat3(N) * iv3normal;
	vv3normal = normalize(normal);

	mat4 mv_matrix = V * M;
	vec4 P = mv_matrix * vec4(iv3vertex, 1.0); 
	vec3 n = normalize(mat3(mv_matrix) * iv3normal);
	vec3 t = normalize(mat3(mv_matrix) * iv3tangent);  
	vec3 b = cross(n, t);   

	matTBN = transpose(mat3(
		t,
		b,
		n
	)); 

	vv3pos = vec3(P) / P.w;

	vv3tangent = iv3tangent;
}