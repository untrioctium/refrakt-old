#version 430

in vec2 pos;

layout(location=0) out vec4 position;
layout(location=1) out vec4 color;

uniform sampler2D last_iter;

uniform vec4 pick;
uniform float angle_pow = 1.0;
uniform float len_pow = 1.0;
uniform float hue_offset = 0.0;
uniform float alpha = 1.0;
uniform float seed = 0.35235234230;
uniform int warmup = 0;

float PHI = 1.61803398874989484820459 * 00000.1; // Golden Ratio   
float PI  = 3.14159265358979323846264 * 00000.1; // PI
float SQ2 = 1.41421356237309504880169 * 10000.0; // Square Root of Two

float gold_noise(in vec2 coordinate, in float seed){
    return fract(tan(distance(coordinate*(seed+PHI), vec2(PHI, PI)))*SQ2);
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

vec3 pickover( vec3 v ) {
	return vec3( sin(pick.x * v.y) - v.z * cos( pick.y * v.x), v.z * sin( pick.z * v.x) - cos( pick.w * v.y ), sin(v.x));
}

void main() {
	vec3 cur, prev, next;
	float lcurnext, lprevcur;

	if(warmup == 1) {
		next = vec3( pos * 200 - 100, gold_noise( pos, seed )* 200 - 100);

		for( int i = 0; i < 20; i++ ) {
			prev = cur;
			cur = next;
			next = pickover(cur);
		}

		lprevcur = distance(cur, prev);
	} else {
		vec4 last = texture(last_iter, pos);
		cur = last.xyz + vec3(gold_noise( pos, seed )) * .01;
		next = pickover( last.xyz );
		lprevcur = last.w;
	}

	lcurnext = distance(cur, next);
	float len = lcurnext + lprevcur;
	len = pow( len/(1 + len), len_pow );
	
	vec3 hsv;
	hsv.r = fract(len +  hue_offset);
	hsv.g = 1 - exp( - hsv.r * hsv.r * 2.5 );
	hsv.b = 1;
	
	position = vec4(next, lcurnext);
	color = vec4(hsvToRGB(hsv), alpha );

}