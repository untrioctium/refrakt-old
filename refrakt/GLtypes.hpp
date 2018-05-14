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

using vec2 = base_vec2<float>;
using ivec2 = base_vec2<int>;
using uvec2 = base_vec2<unsigned int>;
using dvec2 = base_vec2<double>;
using bvec2 = base_vec2<bool>;

using vec3 = base_vec3<float>;
using ivec3 = base_vec3<int>;
using uvec3 = base_vec3<unsigned int>;
using dvec3 = base_vec3<double>;
using bvec3 = base_vec3<bool>;

using vec4 = base_vec4<float>;
using ivec4 = base_vec4<int>;
using uvec4 = base_vec4<unsigned int>;
using dvec4 = base_vec4<double>;
using bvec4 = base_vec4<bool>;