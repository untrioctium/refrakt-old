#version 430
in vec2 pos;
layout(location=0) out vec4 color;

float PHI = 1.61803398874989484820459 * 00000.1; // Golden Ratio   
float PI  = 3.14159265358979323846264 * 00000.1; // PI
float SQ2 = 1.41421356237309504880169 * 10000.0; // Square Root of Two

float gold_noise(in vec2 coordinate, in float seed){
    return fract(tan(distance(coordinate*(seed+PHI), vec2(PHI, PI)))*SQ2);
}

void main() {
	color = vec4( gold_noise(pos, 12445.0), gold_noise(pos, 72445.0), gold_noise(pos, 122355.0), 1.0);
}