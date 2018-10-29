#pragma once

#include <variant>
#include <nlohmann/json.hpp>

#define GLM_FORCE_PURE
#include <glm/glm.hpp>

#include "texture.hpp"

namespace refrakt {

	template<typename T> struct is_static_array {
	private:
		template<std::size_t S, typename U, glm::qualifier Q>
		static auto test(glm::vec<S,U,Q> a)->std::true_type;
		static auto test(...)->std::false_type;
	public:
		static constexpr bool value = decltype(is_static_array::test(T()))::value;
	};

	template<typename T>
	constexpr bool is_static_array_v = is_static_array<T>::value;

	template<typename T> 
	auto type_string(const T& v) -> const std::string { static_assert(std::false_type, "Unknown type"); }

	#define USING_WITH_STATIC_ASSERT( name, t, expected ) \
		using name = t; \
		auto type_string( const name& v ) -> const std::string; \
		static_assert(sizeof(name) == expected, "size mismatch");

	USING_WITH_STATIC_ASSERT(float_t, glm::vec1, 4)
		USING_WITH_STATIC_ASSERT(double_t, glm::dvec1, 8)
		USING_WITH_STATIC_ASSERT(int32_t, glm::ivec1, 4)
		USING_WITH_STATIC_ASSERT(uint32_t, glm::uvec1, 4)

		USING_WITH_STATIC_ASSERT(vec2, glm::vec2, 8)
		USING_WITH_STATIC_ASSERT(dvec2, glm::dvec2, 16)
		USING_WITH_STATIC_ASSERT(ivec2, glm::ivec2, 8)
		USING_WITH_STATIC_ASSERT(uvec2, glm::uvec2, 8)

		USING_WITH_STATIC_ASSERT(vec3, glm::vec3, 12)
		USING_WITH_STATIC_ASSERT(dvec3, glm::dvec3, 24)
		USING_WITH_STATIC_ASSERT(ivec3, glm::ivec3, 12)
		USING_WITH_STATIC_ASSERT(uvec3, glm::uvec3, 12)

		USING_WITH_STATIC_ASSERT(vec4, glm::vec4, 16)
		USING_WITH_STATIC_ASSERT(dvec4, glm::dvec4, 32)
		USING_WITH_STATIC_ASSERT(ivec4, glm::ivec4, 16)
		USING_WITH_STATIC_ASSERT(uvec4, glm::uvec4, 16)

		USING_WITH_STATIC_ASSERT(mat2x2, glm::mat2x2, 16)
		USING_WITH_STATIC_ASSERT(mat2x3, glm::mat2x3, 24)
		USING_WITH_STATIC_ASSERT(mat2x4, glm::mat2x4, 32)
		USING_WITH_STATIC_ASSERT(mat3x2, glm::mat3x2, 24)
		USING_WITH_STATIC_ASSERT(mat3x3, glm::mat3x3, 36)
		USING_WITH_STATIC_ASSERT(mat3x4, glm::mat3x4, 48)
		USING_WITH_STATIC_ASSERT(mat4x2, glm::mat4x2, 32)
		USING_WITH_STATIC_ASSERT(mat4x3, glm::mat4x3, 48)
		USING_WITH_STATIC_ASSERT(mat4x4, glm::mat4x4, 64)

		USING_WITH_STATIC_ASSERT(dmat2x2, glm::dmat2x2, 32)
		USING_WITH_STATIC_ASSERT(dmat2x3, glm::dmat2x3, 48)
		USING_WITH_STATIC_ASSERT(dmat2x4, glm::dmat2x4, 64)
		USING_WITH_STATIC_ASSERT(dmat3x2, glm::dmat3x2, 48)
		USING_WITH_STATIC_ASSERT(dmat3x3, glm::dmat3x3, 72)
		USING_WITH_STATIC_ASSERT(dmat3x4, glm::dmat3x4, 96)
		USING_WITH_STATIC_ASSERT(dmat4x2, glm::dmat4x2, 64)
		USING_WITH_STATIC_ASSERT(dmat4x3, glm::dmat4x3, 96)
		USING_WITH_STATIC_ASSERT(dmat4x4, glm::dmat4x4, 128)

		using arg_t = std::variant<
		float_t, double_t, int32_t, uint32_t,
		vec2, dvec2, ivec2, uvec2,
		vec3, dvec3, ivec3, uvec3,
		vec4, dvec4, ivec4, uvec4,
		mat2x2, mat2x3, mat2x4,
		mat3x2, mat3x3, mat3x4,
		mat4x2, mat4x3, mat4x4,
		dmat2x2, dmat2x3, dmat2x4,
		dmat3x2, dmat3x3, dmat3x4,
		dmat4x2, dmat4x3, dmat4x4,
		texture
		>;

	auto type_string(const texture& arg) -> const std::string;
	auto type_string(const arg_t& arg) -> const std::string;

	using struct_t = std::map<std::string, arg_t>;

	void from_json(const nlohmann::json& j, refrakt::struct_t& s);
}