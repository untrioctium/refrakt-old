#version 430

in vec2 pos;
out vec4 color;

uniform struct roots_t {
	vec4 d[16];
	uint s;
} roots;
uniform float eccentricity = 1.0;

vec2 cInv( vec2 z )
{
	return vec2( z.x, -z.y )/(z.x*z.x + z.y * z.y);
}
vec2 cDiv( vec2 a, vec2 b )
{
	return vec2( a.x * b.x + a.y * b.y, a.y * b.x - a.x * b.y )/(b.x*b.x + b.y * b.y);
}

vec2 newton( vec2 z ) {
	vec2 sum = vec2(0.0);
	for( uint i = 0; i < roots.s; i++ )
		sum += cDiv(roots.d[i].zw, z - roots.d[i].xy);

	return z - eccentricity * cInv(sum);
}

vec3 hsvToRGB( vec3 hsv )
{
	if(hsv.r == 1) hsv.r = 0;
	
	int i = int(floor(hsv.r*6));
	float f = hsv.r*6 - i;
	float p = (hsv.b * (1 - hsv.g));
	float q = hsv.b * (1 - f * hsv.g);
	float t = hsv.b * (1 - (1-f) * hsv.g);
	float v = hsv.b;

	switch(i)
	{
		case 0:
			return vec3(v,t,p);
		case 1:
			return vec3(q,v,p);
		case 2:
			return vec3(p,v,t);
		case 3:
			return vec3(p,q,v);
		case 4:
			return vec3(t,p,v);
		case 5:
			return vec3(v,p,q);
	}
	return vec3(0,0,0);
}

#define PI 3.14159265358979323846264

void main() {
	vec2 proj = pos * 2.0 - 1.0;
	proj.x *= 16.0/9.0;

	const uint MAX_ITERS = 100;
	const float DELTA = .0001;

	vec2 cur = proj;
	vec2 next;
	uint iters = 0;
	for( ; iters < MAX_ITERS; iters++ ) {
		next = newton(cur);
		if( distance(next, cur) < DELTA ) break;
		cur = next;
	}

	float rat = log(float(iters))/log(float(MAX_ITERS));
	float ang = atan(next.y, next.x) + PI;
	ang /= 2 * PI;
	color = vec4( hsvToRGB(vec3(ang, rat, 1 - rat)), 1.0);
}
