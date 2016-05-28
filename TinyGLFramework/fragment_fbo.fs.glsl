#version 410 core

uniform sampler2D tex0;
uniform int renderMode;
uniform float elapsed_time;

layout (location = 0) out vec4 color0;
layout (location = 1) out vec4 color1;
layout (location = 2) out vec4 color2;
layout (location = 3) out vec4 color3;

in VS_OUT 
{ 
	vec2 texcoord; 
} fs_in;

int img_size = 1024;

vec4 grayscale(vec4 color_in)
{
	vec4 tex_color = color_in; 
	float Y = 0.212 * tex_color.r + 0.715 * tex_color.g + 0.072 * tex_color.b; 
	return vec4(Y, Y, Y, 1.0);
}

vec4 blur()
{
	vec4 color = vec4(0); 
	
	int n = 0; 
	int half_size = 2; 
	
	//filter_half_size 
	for ( int i = - half_size; i <= half_size ; ++i ) 
	{        
		for ( int j = - half_size; j <= half_size ; ++j ) 
		{ 
			vec4 c = texture(tex0,fs_in.texcoord + vec2(i, j) / img_size); 
			color += c; 
			n++;
		} 
	} 
	color /= n;
	
	return color;
}

vec4 quantnize()
{
	int nbins = 8; 
	vec4 tex_color = texture(tex0, fs_in.texcoord); 

	float r = floor(tex_color.r * float(nbins)) / float(nbins); 
	float g = floor(tex_color.g * float(nbins)) / float(nbins); 
	float b = floor(tex_color.b * float(nbins)) / float(nbins);

	return vec4(r,g,b,tex_color.a);
}

vec4 dog()
{
	float sigma_e = 2.0f; 
	float sigma_r = 2.8f;
	float phi = 3.4f; 
	float tau = 0.99f;
	float twoSigmaESquared = 2.0 * sigma_e * sigma_e; 
	float twoSigmaRSquared = 2.0 * sigma_r * sigma_r; 
	int halfWidth = int(ceil( 2.0 * sigma_r ));

	vec2 sum = vec2(0.0); 
	vec2 norm = vec2(0.0);
	
	for ( int i = -halfWidth; i <= halfWidth; ++i ) 
	{ 
		for ( int j = -halfWidth; j <= halfWidth; ++j ) 
		{ 
			float d = length(vec2(i,j)); 
			vec2 kernel= vec2( exp( -d * d / twoSigmaESquared ), exp( -d * d / twoSigmaRSquared ));
			vec4 c = texture(tex0, fs_in.texcoord + vec2(i,j) / img_size); 
			vec2 L = vec2(0.299 * c.r + 0.587 * c.g + 0.114 * c.b);
			norm += 2.0 * kernel; 
			sum += kernel * L;
		} 
	}
	sum /= norm; 
	
	float H = 100.0 * (sum.x - tau * sum.y); 
	float edge =( H > 0.0 ) ? 1.0 : 2.0 * smoothstep(-2.0, 2.0, phi * H ); 
	
	return vec4(edge,edge,edge,1.0);
}

vec4 laplacian()
{
	vec4 color = vec4(0);
	
	int n = 0; 
	int half_size = 1; 
	float threshold = 0.01;
	
	for ( int i = - half_size; i <= half_size ; ++i ) 
	{        
		for ( int j = - half_size; j <= half_size ; ++j ) 
		{ 
			vec4 c = grayscale(texture(tex0,fs_in.texcoord + vec2(i, j) / img_size)); 
			color += (i == 0 && j == 0)  ? c * 8.0 : c * -1.0;
 			n++;
		} 
	} 
	
	return (color / n).r > threshold ? vec4(1.0, 1.0, 1.0, 1.0) : vec4(0.0);
}

vec4 sharp()
{
	vec4 color = vec4(0);
	
	int n = 0; 
	int half_size = 1; 
	
	float kernel[9] = float[](
		-1, -1, -1,
		-1, 9, -1,
		-1, -1, -1
	);
	
	for ( int i = - half_size; i <= half_size ; ++i ) 
	{        
		for ( int j = - half_size; j <= half_size ; ++j ) 
		{ 
			vec4 c = texture(tex0,fs_in.texcoord + vec2(i, j) / img_size); 
			color += c * kernel[n];
 			n++;
		} 
	} 
	
	return color;
}

vec4 pixelize()
{
	vec4 color = vec4(0);
	
	int n = 0; 
	int half_size = 1; 
	int pixel_size = 128;

	vec2 pos = floor(fs_in.texcoord * pixel_size) / pixel_size;

	for ( int i = - half_size; i <= half_size ; ++i ) 
	{        
		for ( int j = - half_size; j <= half_size ; ++j ) 
		{ 
			vec4 c = texture(tex0, pos + vec2(i, j) / img_size); 
			color += c;
 			n++;
		} 
	} 
	
	return color / n;
}

float frequency = 1000.0; // Needed globally for lame version of aastep()

float aastep(float threshold, float value) 
{
	float afwidth = 0.7 * length(vec2(dFdx(value), dFdy(value)));
	return smoothstep(threshold - afwidth, threshold + afwidth, value);
}

vec4 halftone()
{	
	// Distance to nearest point in a grid of
    // (frequency x frequency) points over the unit square
	vec2 st = fs_in.texcoord;

    vec2 st2 = mat2(0.707, -0.707, 0.707, 0.707) * st;
    vec2 nearest = 2.0*fract(frequency * st2) - 1.0;
    float dist = length(nearest);

    // Use a texture to modulate the size of the dots
    vec3 texcolor = texture2D(tex0, st).rgb; // Unrotated coords
    float radius = sqrt(1.0-texcolor.g); // Use green channel

    vec3 white = vec3(1.0, 1.0, 1.0);
    vec3 black = vec3(0.0, 0.0, 0.0);
    vec3 fragcolor = mix(black, white, aastep(radius, dist));

    return vec4(fragcolor, 1.0);
}

vec4 radial_blur()
{
	const float sampleDist = 1.0;
	const float sampleStrength = 2.5; 

	float samples[21] = float[](
		-0.1, -0.09, -0.08, -0.07, -0.06, -0.05, -0.04, -0.03, -0.02, -0.01,
		0.0,
		0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.1
	);

	vec2 uv = fs_in.texcoord;
    vec2 dir = 0.5 - uv; 
    float dist = length(dir);
    dir = normalize(dir);

    vec4 color = texture2D(tex0,uv); 
    vec4 sum = color;

	int n = 21;
    for (int i = 0; i < n; i++)
        sum += texture2D( tex0, uv + dir * samples[i] * sampleDist );
		
    sum *= 1.0 / n;

    float t = dist * sampleStrength;
    t = clamp(t ,0.0, 1.0);

	return mix( color, sum, t );
}

vec4 ripple()
{
		float time = (cos(elapsed_time));
		vec2 tc = fs_in.texcoord.xy;
		vec2 p = -1.0 + 2.0 * tc;
	 
		float len = length(p);
		vec2 uv = tc + (p / len) * cos(len * 12.0 - time * 4.0) * 0.03;
		vec3 col = texture2D(tex0, uv).xyz;
		
		return vec4(col,1.0);  
}
void main(void) 
{ 
	color0 = texture(tex0, fs_in.texcoord);
	if(renderMode == 1)
	{
		color1 = blur();
		color2 = quantnize();
		color3 = dog();
	}
	else if(renderMode == 2)
	{
		color1 = laplacian();
		color2 = vec4(1.0, 1.0, 1.0, 1.0);
		color3 = vec4(1.0, 1.0, 1.0, 1.0);
	}
	else if(renderMode == 3)
	{
		color1 = sharp();
		color2 = vec4(1.0, 1.0, 1.0, 1.0);
		color3 = vec4(1.0, 1.0, 1.0, 1.0);
	}
	else if(renderMode == 4)
	{
		color1 = pixelize();
		color2 = vec4(1.0, 1.0, 1.0, 1.0);
		color3 = vec4(1.0, 1.0, 1.0, 1.0);
	}
	else if(renderMode == 5)
	{
		color1 = blur();
		color2 = vec4(1.0, 1.0, 1.0, 1.0);
		color3 = vec4(1.0, 1.0, 1.0, 1.0);
	}
	else if(renderMode == 6)
	{
		color1 = halftone();
		color2 = vec4(1.0, 1.0, 1.0, 1.0);
		color3 = vec4(1.0, 1.0, 1.0, 1.0);
	}
	else if(renderMode == 7)
	{
		color1 = radial_blur();
		color2 = vec4(1.0, 1.0, 1.0, 1.0);
		color3 = vec4(1.0, 1.0, 1.0, 1.0);
	}
	else if(renderMode == 8)
	{
		color1 = ripple();
		color2 = vec4(1.0, 1.0, 1.0, 1.0);
		color3 = vec4(1.0, 1.0, 1.0, 1.0);
	}
	else
	{
		color1 = color2 = color3 = color0 = grayscale(color0);
	}
}