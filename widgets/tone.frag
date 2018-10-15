#version 430
in vec2 pos;
uniform sampler2D col;
layout(location=0) out vec4 result;

uniform float pre_gamma;
uniform float post_gamma;
uniform float exposure;

void main() {
	vec4 input = texture(col, pos);
	if( input.w < 1.0 ) input.w = 1.0;

	input = pow(input, vec4(pre_gamma));
	input *= exposure;
	input = vec4( input.xyz * log(input.w + 1.0)/input.w, 1.0);
	input = clamp( input, 0, 1 );
	input = pow(input, vec4(post_gamma));
	result = vec4( input.xyz, 1.0 );
}