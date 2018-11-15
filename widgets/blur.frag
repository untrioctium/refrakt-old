#version 430

in vec2 pos;
out vec4 color;

uniform sampler2D in1;
uniform sampler2D in2;
uniform sampler2D in3;

void main() {
	color = vec4(	(texture(in1, pos).xyz +  texture(in2, pos).xyz +  texture(in3, pos).xyz) / 3.0, 1.0);
}