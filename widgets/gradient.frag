#version 430
in vec2 pos;
uniform float brightness;
layout(location=0) out vec4 color;

void main() {
	color = vec4( pos.x * brightness, pos.x* brightness, pos.y* brightness, 1.0);
}