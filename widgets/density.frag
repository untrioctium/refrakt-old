#version 430
in vec2 pos;

uniform sampler2D tex;
layout(location = 0) out vec4 result;

uniform unsigned int max_width;
uniform float alpha;
uniform float sig;

#define PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062

float gaussian(float x, float y, float sigma) {
	float s = 2.0 * sigma * sigma;
	return exp(-(x * x + y * y) / s) / sqrt(s * PI);
}

void main() {
	vec4 point = texture(tex, pos);
	ivec2 size = textureSize(tex, 0);
	float dx = 1.0/float(size.x);
	float dy = 1.0/float(size.y);

	int width = int(max_width / pow(point.a + 1.0, alpha));
	if( width <= 1 ) {
		result = point; 
		return;
	}
	if( width > max_width ) width = int(max_width);

	int radius = width / 2;

	float kernel_sum = 0.0;
	vec4 total_sum = vec4(0.0);
	int total = 0;
	for( int x = -radius; x <= radius; x++ ) {
		for( int y = -radius; y <= radius; y++ )
		{
			float gauss = gaussian(x, y, sig);
			kernel_sum += gauss;
			total_sum += gauss * texture(tex, pos + vec2( dx * float(x), dy * float(y)));
			total +=1;
		}
	}

	result = total_sum / (total * kernel_sum);
}