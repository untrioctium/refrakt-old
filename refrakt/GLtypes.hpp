#pragma once
#include <variant>

// inheritable struct template that signifies the child
// should always have a constant size in bytes
template<std::size_t s> struct const_sized {
	static constexpr std::size_t type_size = s;
};

// checks to see if a type is const_sized or a basic integral type
// is_const_sized<vec2>::value is true, but is_const_sized<std::string>::value is false
template<typename T> struct is_const_sized {
private:
	template<std::size_t s>
	static std::true_type test(const_sized<s> v) { return std::true_type(); };
	static std::false_type test(...) { return std::false_type(); };

public:
	static constexpr bool value = std::is_integral_v<T> || decltype(is_const_sized::test(T()))::value;
};

// specialized overload because std::float_t does not return true with is_integral_v
template<> struct is_const_sized<std::float_t> {
	static constexpr bool value = true;
};

// specialized overload because std::double_t does not return true with is_integral_v
template<> struct is_const_sized<std::double_t> {
	static constexpr bool value = true;
};

template<typename T, std::size_t s> struct ConstSizeArray : public const_sized<sizeof(T) * s> {
public:
	T & operator[](std::size_t index) { return *(data_ + index); }
	T* operator&() { return data_; }
	T& operator*() { return data_; }

	ConstSizeArray() {}
	ConstSizeArray(T init) { for (std::size_t index = 0; index < len; index++) data_[index] = init;}
	ConstSizeArray(std::initializer_list<T> l) { for (std::size_t index = 0; index < len; index++) data_[index] = *(l.begin() + index); }

	ConstSizeArray<T, s>& operator=(ConstSizeArray<T, s> that) {
		for (std::size_t index = 0; index < len; index++) data_[index] = that.data_[index];
		return *this;
	}

	template<std::size_t index> T get() { return data_[index]; }
	template<std::size_t index> void set(const T value) { data_[index] = value; }

	static constexpr std::size_t len = s;
	typedef T stored_type;

private:
	T data_[s] = { 0 };
};

#define USING_WITH_STATIC_ASSERT( name, t, size, expected ) \
	using name = ConstSizeArray<t, size>; \
	static_assert( is_const_sized<t>::value, "'" #name "' groups a non-const_sized type '" #t "'"); \
	static_assert( is_const_sized<name>::value, "'" #name "' does not inherit const_sized"); \
	static_assert( sizeof(name) == name::type_size, "internal size mismatch for '" #name "'"); \
	static_assert( name::type_size == expected, "size mismatch for '" #name "' (expected " #expected ")");

// I'm going to regret this later I'm sure.
namespace rfkt
{
		USING_WITH_STATIC_ASSERT(float_t, std::float_t, 1, 4)
		USING_WITH_STATIC_ASSERT(double_t, std::double_t, 1, 8)
		USING_WITH_STATIC_ASSERT(int32_t, std::int32_t, 1, 4)
		USING_WITH_STATIC_ASSERT(uint32_t, std::uint32_t, 1, 4)
		USING_WITH_STATIC_ASSERT(bool_t, bool, 1, 1)
}
USING_WITH_STATIC_ASSERT(vec2, std::float_t, 2, 8)
USING_WITH_STATIC_ASSERT(dvec2, std::double_t, 2, 16)
USING_WITH_STATIC_ASSERT(ivec2, std::int32_t, 2, 8)
USING_WITH_STATIC_ASSERT(uvec2, std::uint32_t, 2, 8)
USING_WITH_STATIC_ASSERT(bvec2, bool, 2, 2)

USING_WITH_STATIC_ASSERT(vec3, std::float_t, 3, 12)
USING_WITH_STATIC_ASSERT(dvec3, std::double_t, 3, 24)
USING_WITH_STATIC_ASSERT(ivec3, std::int32_t, 3, 12)
USING_WITH_STATIC_ASSERT(uvec3, std::uint32_t, 3, 12)
USING_WITH_STATIC_ASSERT(bvec3, bool, 3, 3)

USING_WITH_STATIC_ASSERT(vec4, std::float_t, 4, 16)
USING_WITH_STATIC_ASSERT(dvec4, std::double_t, 4, 32)
USING_WITH_STATIC_ASSERT(ivec4, std::int32_t, 4, 16)
USING_WITH_STATIC_ASSERT(uvec4, std::uint32_t, 4, 16)
USING_WITH_STATIC_ASSERT(bvec4, bool, 4, 4)

USING_WITH_STATIC_ASSERT(mat2x2, vec2, 2, 16)
USING_WITH_STATIC_ASSERT(mat2x3, vec2, 3, 24)
USING_WITH_STATIC_ASSERT(mat2x4, vec2, 4, 32)
USING_WITH_STATIC_ASSERT(mat3x2, vec3, 2, 24)
USING_WITH_STATIC_ASSERT(mat3x3, vec3, 3, 36)
USING_WITH_STATIC_ASSERT(mat3x4, vec3, 4, 48)
USING_WITH_STATIC_ASSERT(mat4x2, vec4, 2, 32)
USING_WITH_STATIC_ASSERT(mat4x3, vec4, 3, 48)
USING_WITH_STATIC_ASSERT(mat4x4, vec4, 4, 64)

USING_WITH_STATIC_ASSERT(dmat2x2, dvec2, 2, 32)
USING_WITH_STATIC_ASSERT(dmat2x3, dvec2, 3, 48)
USING_WITH_STATIC_ASSERT(dmat2x4, dvec2, 4, 64)
USING_WITH_STATIC_ASSERT(dmat3x2, dvec3, 2, 48)
USING_WITH_STATIC_ASSERT(dmat3x3, dvec3, 3, 72)
USING_WITH_STATIC_ASSERT(dmat3x4, dvec3, 4, 96)
USING_WITH_STATIC_ASSERT(dmat4x2, dvec4, 2, 64)
USING_WITH_STATIC_ASSERT(dmat4x3, dvec4, 3, 96)
USING_WITH_STATIC_ASSERT(dmat4x4, dvec4, 4, 128)