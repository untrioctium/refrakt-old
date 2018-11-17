#pragma once

#include <variant>
#include <nlohmann/json.hpp>

#define GLM_FORCE_PURE
#include <glm/glm.hpp>

#include "texture.hpp"
#include "fixed_vector.hpp"

namespace refrakt {

	template<typename T>
	using array_type = refrakt::fixed_vector<T>;

	namespace detail {
		template<typename T> struct is_vector_type_impl {
		private:
			template<std::size_t S, typename U, glm::qualifier Q>
			static auto test(glm::vec<S, U, Q> a)->std::true_type;
			static auto test(...)->std::false_type;
		public:
			static constexpr bool value = decltype(is_vector_type_impl::test(T()))::value;
		};

		template<typename T> struct is_matrix_type_impl {
		private:
			template<std::size_t C, std::size_t R, typename U, glm::qualifier Q>
			static auto test(glm::mat<C, R, U, Q> a)->std::true_type;
			static auto test(...)->std::false_type;
		public:
			static constexpr bool value = decltype(is_matrix_type_impl::test(T()))::value;
		};

		template<typename T> struct is_array_type_impl {
		private:
			template<typename T>
			static auto test(array_type<T> a)->std::true_type;
			static auto test(...)->std::false_type;
		public:
			static constexpr bool value = decltype(is_array_type_impl::test(T()))::value;
		};
	}

	template<typename T>
	constexpr bool is_vector_type = detail::is_vector_type_impl<T>::value;

	template<typename T>
	constexpr bool is_matrix_type = detail::is_matrix_type_impl<T>::value;

	template<typename T>
	constexpr bool is_array_type = detail::is_array_type_impl<T>::value;

	template<typename T> 
	auto type_string(const T& v) -> const std::string { static_assert(std::false_type, "Unknown type"); }


	#define USING_WITH_STATIC_ASSERT( name, t, expected ) \
		using name = t; \
		using name##_array = array_type<name>; \
		auto type_string( const name& v ) -> const std::string; \
		auto type_string( const name##_array& v ) -> const std::string; \
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

		using texture_array = array_type<texture>;

#define ADD_TO_ARG_T_VARIANT(name) name, name##_array

		using arg_t = std::variant<
		ADD_TO_ARG_T_VARIANT(float_t), ADD_TO_ARG_T_VARIANT(double_t), ADD_TO_ARG_T_VARIANT(int32_t), ADD_TO_ARG_T_VARIANT(uint32_t),
		ADD_TO_ARG_T_VARIANT(vec2), ADD_TO_ARG_T_VARIANT(dvec2), ADD_TO_ARG_T_VARIANT(ivec2), ADD_TO_ARG_T_VARIANT(uvec2),
		ADD_TO_ARG_T_VARIANT(vec3), ADD_TO_ARG_T_VARIANT(dvec3), ADD_TO_ARG_T_VARIANT(ivec3), ADD_TO_ARG_T_VARIANT(uvec3),
		ADD_TO_ARG_T_VARIANT(vec4), ADD_TO_ARG_T_VARIANT(dvec4), ADD_TO_ARG_T_VARIANT(ivec4), ADD_TO_ARG_T_VARIANT(uvec4),
		ADD_TO_ARG_T_VARIANT(mat2x2), ADD_TO_ARG_T_VARIANT(mat2x3), ADD_TO_ARG_T_VARIANT(mat2x4),
		ADD_TO_ARG_T_VARIANT(mat3x2), ADD_TO_ARG_T_VARIANT(mat3x3), ADD_TO_ARG_T_VARIANT(mat3x4),
		ADD_TO_ARG_T_VARIANT(mat4x2), ADD_TO_ARG_T_VARIANT(mat4x3), ADD_TO_ARG_T_VARIANT(mat4x4),
		ADD_TO_ARG_T_VARIANT(dmat2x2), ADD_TO_ARG_T_VARIANT(dmat2x3), ADD_TO_ARG_T_VARIANT(dmat2x4),
		ADD_TO_ARG_T_VARIANT(dmat3x2), ADD_TO_ARG_T_VARIANT(dmat3x3), ADD_TO_ARG_T_VARIANT(dmat3x4),
		ADD_TO_ARG_T_VARIANT(dmat4x2), ADD_TO_ARG_T_VARIANT(dmat4x3), ADD_TO_ARG_T_VARIANT(dmat4x4),
		ADD_TO_ARG_T_VARIANT(texture)
		>;

	auto type_string(const texture& arg) -> const std::string;
	auto type_string(const arg_t& arg) -> const std::string;

	using struct_t = std::map<std::string, arg_t>;

	void from_json(const nlohmann::json& j, refrakt::struct_t& s);
}