#version 430
in vec2 pos;
layout(location=0) out vec3 result;

float PHI = 1.61803398874989484820459 * 00000.1; // Golden Ratio   
float PI  = 3.14159265358979323846264 * 00000.1; // PI
float SQ2 = 1.41421356237309504880169 * 10000.0; // Square Root of Two

uniform float prob;

float gold_noise(in vec2 coordinate, in float seed){
    return fract(tan(distance(coordinate*(seed+PHI), vec2(PHI, PI)))*SQ2);
}

vec2 fern( vec2 p, float f ) {
	if( f <= 0.01 ) return vec2( 0.0, 0.16 * p.y );
	if( f <= 0.86 ) return vec2( 0.85 * p.x + 0.04 * p.y, -0.04 * p.x + 0.85 * p.y + 1.6);
	if( f <= 0.93 ) return vec2( 0.2 * p.x - 0.26 * p.y, 0.23 * p.x + 0.22 * p.y + 1.6);
	return vec2( -0.15 * p.x + 0.28 * p.y, 0.26 * p.x + 0.24 * p.y + 0.44 );
}

vec3 dragon(vec3 p, float f) {
	if( f <= prob ) return vec3( 0.824074 * p.x + 0.281428 * p.y + -1.882290, -0.212346 * p.x + 0.864198 * p.y + -0.110607, p.z * 0.5 + 0.5 );
	return vec3( 0.088272 * p.x + 0.520988 * p.y + 0.785360, -0.463889 * p.x + -0.377778 * p.y + 8.095795, p.z * 0.5 );
}

void main() {
	vec3 p = vec3(pos, gold_noise(pos, 7.0));

	for( int i = 0; i < 50; i++ ) p = dragon(p, gold_noise(p.xy, dot(p.xy, pos) + 3.0));

	result = vec3(p.x/ 8.0, p.y/ 8.0 - 0.5, p.z);
}