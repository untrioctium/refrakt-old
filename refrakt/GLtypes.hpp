#pragma once

// inheritable struct template that signifies the child
// should always have a constant size in bytes
template<std::size_t s> struct const_sized {
	static constexpr std::size_t type_size = s;
};

template<typename T> struct is_const_sized {
private:
	template<std::size_t s>
	static std::true_type test(const_sized<s> v) { return std::true_type(); };
	static std::false_type test(...) { return std::false_type();  };

public:
	static constexpr bool value = std::is_integral_v<T> || decltype(is_const_sized::test(T()))::value;
};

template<typename T> struct vec2_base: public const_sized<sizeof(T) * 2> {
	T x, y;
	
	vec2_base() {}
	vec2_base(T X, T Y) : x(X), y(Y) {}
	vec2_base(T val) : vec2_base(val, val) {}

	using unique_constructor = vec2_base<T>(T,T);
};

template<typename T> struct vec3_base {
	T x, y, z;

	vec3_base() {}
	vec3_base(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
	vec3_base(T val) : vec3_base(val, val, val) {}

	using unique_constructor = vec3_base<T>(T, T, T);
	static constexpr std::size_t type_size = sizeof(T) * 3;
};

template<typename T> struct vec4_base {
	T x, y, z, w;

	vec4_base() {}
	vec4_base(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}
	vec4_base(T val) : vec4_base(val, val, val, val) {}

	using unique_constructor = vec4_base<T>(T, T, T, T);
	static constexpr std::size_t type_size = sizeof(T) * 4;
};

// macro to create templated types with enforced expected size
#define USING_WITH_STATIC_SIZE( type, templ, expected ) \
	using type = templ; \
	static_assert( sizeof(type) == expected && type::type_size == expected, "size mismatch for '" #type "' (expected " #expected ")" );

USING_WITH_STATIC_SIZE(vec2, vec2_base<std::float_t>, 8)
USING_WITH_STATIC_SIZE(dvec2, vec2_base<std::double_t>, 16)
USING_WITH_STATIC_SIZE(ivec2, vec2_base<std::int32_t>, 8)
USING_WITH_STATIC_SIZE(uvec2, vec2_base<std::uint32_t>, 8)
USING_WITH_STATIC_SIZE(bvec2, vec2_base<bool>, 2)

USING_WITH_STATIC_SIZE(vec3, vec3_base<std::float_t>, 12)
USING_WITH_STATIC_SIZE(dvec3, vec3_base<std::double_t>, 24)
USING_WITH_STATIC_SIZE(ivec3, vec3_base<std::int32_t>, 12)
USING_WITH_STATIC_SIZE(uvec3, vec3_base<std::uint32_t>, 12)
USING_WITH_STATIC_SIZE(bvec3, vec3_base<bool>, 3)

USING_WITH_STATIC_SIZE(vec4, vec4_base<std::float_t>, 16)
USING_WITH_STATIC_SIZE(dvec4, vec4_base<std::double_t>, 32)
USING_WITH_STATIC_SIZE(ivec4, vec4_base<std::int32_t>, 16)
USING_WITH_STATIC_SIZE(uvec4, vec4_base<std::uint32_t>, 16)
USING_WITH_STATIC_SIZE(bvec4, vec4_base<bool>, 4)

static_assert(is_const_sized<vec2>::value, "vec2 is not const sized!");
static_assert(is_const_sized<std::string>::value == false, "std::string shouldn't be const sized");