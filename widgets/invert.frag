#version 430
in vec2 pos;
uniform sampler2D input;

layout(location=0) out vec4 color;

void main() {
	color = vec4( 1.0 - texture(input,pos).xyz, 1.0 );
}