#version 410

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec3 iv3normal;
layout(location = 2) in vec2 iv2texcoord;
layout(location = 3) in vec3 iv3tangent;

uniform mat4 mvp;
uniform mat4 M, V, P;
uniform vec3 E;
uniform mat4 lightSpaceMatrix;
uniform int objectVariant;
uniform int isReflect;
uniform float waterHeight;

out vec4 vv4color;
out vec3 vv3normal;
out vec3 vv3tangent;
out vec3 vv3pos;
out vec2 vv2texcoord;
out vec3 vv3texcoord;
out mat3 matTBN;
out vec3 worldPos;
out vec4 vv4posLightSpace;

void main()
{
	if(objectVariant == -1)
	{
		gl_Position = P * V * vec4(iv3vertex + E, 1.0);
		vv3texcoord = iv3vertex;
	}
	else 
	{
		mat4 N = transpose(inverse(V * M));
		gl_Position = mvp * vec4(iv3vertex, 1.0);
		vv2texcoord = iv2texcoord;

		vec3 normal = normalize(mat3(N) * iv3normal);
		vv3normal = normal;

		mat4 mv_matrix = V * M;
		vec4 P = mv_matrix * vec4(iv3vertex, 1.0); 
		vec3 n = normal;
		vec3 t = normalize(mat3(N) * iv3tangent);  
		vec3 b = cross(n, t);   

		matTBN = inverse(mat3(
			t,
			b,
			n
		)); 

		vv3pos = vec3(P) / P.w;

		vv3tangent = iv3tangent;
		worldPos = vec3(M * vec4(iv3vertex, 1.0));
		vv4posLightSpace = lightSpaceMatrix * vec4(worldPos, 1.0);

		if(isReflect == 1)
		{
			vec4 plane = vec4( 0, 1, 0, waterHeight);
			gl_ClipDistance[0] = dot(vec4(worldPos, 1.0) , plane);
		}
	}
}
