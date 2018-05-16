#pragma once

template<typename T> struct base_vec2 {
	T x, y;
	base_vec2() {}
	base_vec2(T X, T Y) : x(X), y(Y) {}
	base_vec2(T val) : base_vec2(val, val) {}
};

template<typename T> struct base_vec3 {
	T x, y, z;

	base_vec3() {}
	base_vec3(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
	base_vec3(T val) : base_vec3(val, val, val) {}
};

template<typename T> struct base_vec4 {
	T x, y, z, w;

	base_vec4() {}
	base_vec4(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}
	base_vec4(T val) : base_vec4(val, val, val, val) {}
};

#define USING_WITH_STATIC_SIZE( clsname, prefix, type, expected ) \
	using prefix##clsname = base_##clsname<type>; \
	static_assert( sizeof(prefix##clsname) * 8 == expected, "size mismatch for '" #prefix #clsname "' (expected " #expected ")" );

USING_WITH_STATIC_SIZE(vec2,  , float, 64)
USING_WITH_STATIC_SIZE(vec2, i, int, 64)
USING_WITH_STATIC_SIZE(vec2, u, unsigned int, 64)
USING_WITH_STATIC_SIZE(vec2, d, double, 128)
USING_WITH_STATIC_SIZE(vec2, b, bool, 16)

USING_WITH_STATIC_SIZE(vec3,  , float, 96)
USING_WITH_STATIC_SIZE(vec3, i, int, 96)
USING_WITH_STATIC_SIZE(vec3, u, unsigned int, 96)
USING_WITH_STATIC_SIZE(vec3, d, double, 192)
USING_WITH_STATIC_SIZE(vec3, b, bool, 24)

USING_WITH_STATIC_SIZE(vec4,  , float, 128)
USING_WITH_STATIC_SIZE(vec4, i, int, 128)
USING_WITH_STATIC_SIZE(vec4, u, unsigned int, 128)
USING_WITH_STATIC_SIZE(vec4, d, double, 256)
USING_WITH_STATIC_SIZE(vec4, b, bool, 32)