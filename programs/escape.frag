#version 430
in vec2 texCoord;
out vec4 color;

struct Hue {
	float shift;
	float stretch;
};

uniform vec2 center;
uniform float scale;
uniform vec2 exponent;
uniform float escape_radius;
uniform Hue hue;
uniform uint max_iterations;
uniform vec2 julia;
uniform vec2 julia_c;
uniform vec2 burning_ship;
uniform float surface_ratio;
uniform vec2 offset;
uniform uint hq_mode;
uniform float time;


float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm (in vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
        value += amplitude * abs(noise(st));
        st *= 2.;
        amplitude *= .5;
    }
    return value;
}

/**
 * @file complex.frag
 * @brief A set of functions for dealing with complex numbers in GLSL.
 */
#define PI 3.14159265358979323846264

/**
 * @brief Adds two complex numbers.
 * @param a The first complex number.
 * @param b The second complex number.
 * @return \f$a + b\f$
 *
 * This function is technically not needed, as the addition of vectors
 * is equivilent to the addition of complex numbers.
 */
vec2 cAdd( vec2 a, vec2 b )
{
	return a + b;
}

/**
 * @brief Subtracts two complex numbers.
 * @param a The minuend.
 * @param b The subtrahend.
 * @return \f$a - b\f$
 *
 * This function is technically not needed, as the subtraction of vectors
 * is equivilent to the addition of complex numbers.
 */
vec2 cSub( vec2 a, vec2 b )
{
	return a - b;
}

/**
 * @brief Multiplies two complex numbers.
 * @param a The first complex number.
 * @param b The second complex number.
 * @return \f$a * b\f$
 */
vec2 cMul( vec2 a, vec2 b )
{
	return vec2( a.x * b.x - a.y * b.y, a.y * b.x + a.x * b.y );
}

/**
 * @brief Calculates the inverse of a complex number.
 * @param z The complex number.
 * @return \f$\frac{1}{z}\f$
 *
 * Use this instead of cDiv if you only wish to find the inverse,
 * as this will be faster in the case where the numerator is 1.
 */
vec2 cInv( vec2 z )
{
	return vec2( z.x, -z.y )/(z.x*z.x + z.y * z.y);
}

/**
 * @brief Divides two complex numbers.
 * @param a The numerator.
 * @param b The denominator.
 * @return \f$\frac{a}{b}\f$
 */
vec2 cDiv( vec2 a, vec2 b )
{
	return vec2( a.x * b.x + a.y * b.y, a.y * b.x - a.x * b.y )/(b.x*b.x + b.y * b.y);
}

/**
 * @brief Calculates complex exponentation.
 * @param a The base.
 * @param b The exponent.
 * @return \f$a^b\f$
 */
vec2 cPow( vec2 a, vec2 b )
{
	float r = sqrt(a.x * a.x + a.y * a.y);
	float theta = atan(a.y, a.x);

	float arg = b.y * log(r) + b.x * theta;
	return pow( r, b.x ) * exp( -b.y * theta ) * vec2( cos(arg), sin(arg) ); 
}

/**
 * @brief Calculates complex exponentation with a real exponent.
 * @param a The base.
 * @param b The exponent.
 * @return \f$a^b\f$
 *
 * Use this overload if you know the exponent will be real, as it shaves off
 * a few calculations versus raising to a complex power.
 */
vec2 cPow( vec2 a, float b )
{
	float arg = b * atan(a.y, a.x);
	return pow( sqrt(a.x * a.x + a.y * a.y), b ) * vec2( cos(arg), sin(arg) );
}

/**
 * @brief Calculates complex exponentation with a real base.
 * @param a The base.
 * @param b The exponent.
 * @return \f$a^b\f$
 *
 * Use this overload if you know the base will be real, as it shaves off
 * a few calculations versus using a complex base.
 */
vec2 cPow( float a, vec2 b )
{
	float r = abs(a);

	float arg = b.y * log(r);
	return pow( r, b.x ) * vec2( cos(arg), sin(arg) );
}

/**
 * @brief Calculates complex exponentation in base \f$e\f$.
 * @param z The exponent.
 * @return \f$e^z\f$
 */
vec2 cExp( vec2 z )
{
	return exp(z.x) * vec2( cos(z.y), sin(z.y) ); 	
}

/**
 * @brief Calculates the complex logarithm base \f$e\f$.
 * @param z The complex number.
 * @return \f$log_e(z)\f$
 */
vec2 cLog( vec2 z )
{
	return vec2( log( z.x * z.x + z.y * z.y ), atan( z.y, z.x ) );
}

/**
 * @brief Calculates the complex sine.
 * @param z The complex number.
 * @return \f$sin(z)\f$
 */
vec2 cSin( vec2 z )
{
	return vec2( sin(z.x) * cosh(z.y), cos(z.x) * sinh(z.y));
}

/**
 * @brief Calculates the complex cosine.
 * @param z The complex number.
 * @return \f$cos(z)\f$
 */
vec2 cCos( vec2 z )
{
	return vec2( cos(z.x) * cosh(z.y), -sin(z.x) * sinh(z.y));
}

/**
 * @brief Calculates the commplex tangent.
 * @param z The complex number.
 * @return \f$tan(z)\f$
 */
vec2 cTan( vec2 z )
{
	return cDiv( cSin(z), cCos(z) );
}

/**
 * @brief Calculates the magnitude of a complex number.
 * @param z The complex number.
 * @return The magnitude of \a z.
 */
float cAbs( vec2 z )
{
	return sqrt( z.x * z.x + z.y * z.y );
}

/**
 * @brief Calculates the argument of a complex number.
 * @param z The complex number.
 * @return The angle made by \a z and the origin, in the range \f$[-\pi,\pi]\f$.
 */
float cArg( vec2 z )
{
	return atan( z.y, z.x );
}

vec2 cToPolar( vec2 z )
{
	return vec2( length(z), cArg(z) );
}

vec2 vlerp( vec2 a, vec2 b, vec2 mix )
{
	return vec2( a.x * (1 - mix.x) + b.x * mix.x, a.y * (1 - mix.y) + b.y * mix.y );
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

vec3 mandelbrot( vec2 position )
{
	vec2 v = position;
	
	float escape = escape_radius * escape_radius;
    float r2 = 0.0;
	
	uint iter = 0;
	
	vec2 offset = vlerp(position, julia_c, julia);

	float min_distance = 0;
    for (; iter < max_iterations && r2 < escape; ++iter)
    {
        v = vlerp(cPow(v, exponent) + offset, cPow(vec2(abs(v.x), abs(v.y)), exponent) + offset, burning_ship);
        r2 = v.x * v.x + v.y * v.y;
		vec2 st = v* .5 + .5;
		st *= 3.0;

		    vec3 color = vec3(0.0);

		 vec2 q = vec2(0.);
		 q.x = fbm( st + 0.01*time);
		 q.y = fbm( st + vec2(1.0));

		vec2 r = vec2(0.);
		r.x = fbm( st + 1.0*q + vec2(1.7,9.2)+ 0.15*time );
		r.y = fbm( st + 1.0*q + vec2(8.3,2.8)+ 0.126*time);

		float f = 1 - fbm(st+r);

		min_distance = max(f, min_distance);
    }
	
	min_distance = sqrt(min_distance);

	float smooth_iter = float(iter) - log(log(sqrt(r2)) / log(escape_radius)) / log(cAbs(exponent));
	float lograt = log(smooth_iter)/log(float(max_iterations));
    if (r2 < escape)
        return vec3(1.0f, 1.0f, 1.0f);
    else {
		float hue = fract(mix(lograt, min_distance, .25) * hue.stretch + hue.shift);
		return hsvToRGB(vec3(hue, 1 - lograt * min_distance,  min_distance * lograt));
	}
}

vec2 project_to_plane( vec2 coord ) {
	vec2 proj = vec2( coord.x * (1.0/scale) * surface_ratio, coord.y * (1.0/scale) * -1) + center;
	return proj;
}

void main() {

	vec2 coord = texCoord.xy;

	if( hq_mode != 0 )
	{
		float y_off = offset.y / 4.0;
		float x_off = offset.y / 4.0;

		vec3 result = mandelbrot( project_to_plane( coord + vec2(x_off, y_off) ) ) + 
					  mandelbrot( project_to_plane( coord + vec2(-x_off, y_off) ) ) +
					  mandelbrot( project_to_plane( coord + vec2(-x_off, -y_off) ) ) +
					  mandelbrot( project_to_plane( coord + vec2(x_off, -y_off) ) );

		color = vec4(result * .25, 1.0);
     } else color = vec4( mandelbrot( project_to_plane( coord )), 1.0);

}