#version 430
uniform float roll;
writeonly uniform image2D destTex;
layout (local_size_x = 16, local_size_y = 16) in;

void main() {
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	
	float real = (float(gl_GlobalInvocationID.x) - 256.0)/256.0;
	float imag = (float(gl_GlobalInvocationID.y) - 256.0)/256.0;
	float Creal = real; 
    float Cimag = imag;

    float r2 = 0.0;

	
	int maxIter = int(roll);
	int iter = 0;
    for (; iter < maxIter && r2 < 16.0; ++iter)
    {
        float tempreal = real;

        real = (tempreal * tempreal) - (imag * imag) + Creal;
        imag = 2.0 * tempreal * imag + Cimag;
        r2   = (real * real) + (imag * imag);
    }

    vec3 color;

    if (r2 < 4.0)
        color = vec3(0.0f, 0.0f, 0.0f);
    else
        color = vec3(log(float(iter))/log(float(maxIter)), 0.0f, 0.0f);
	
	imageStore(destTex, storePos, vec4(color, 1.0));
}