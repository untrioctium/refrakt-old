#pragma once
#include <variant>
#include "json.hpp"

namespace refrakt {

	template<class T, std::size_t s>
	using static_array_t = std::array<T, s>;

	template<typename T> struct is_static_array {
	private:
		template<typename U, std::size_t s>
		static auto test(static_array_t<U, s> a)->std::true_type;
		static auto test(...)->std::false_type;
	public:
		static constexpr bool value = decltype(is_static_array::test(T()))::value;
	};

	template<typename T>
	constexpr bool is_static_array_v = is_static_array<T>::value;

	template<typename T> 
	auto type_string(const T& v) -> const std::string { static_assert(std::false_type, "Unknown type"); }

#define USING_WITH_STATIC_ASSERT( name, t, size, expected ) \
		using name = static_array_t<t, size>; \
		auto type_string( const name& v ) -> const std::string; \
		static_assert(sizeof(name) == expected, "size mismatch");

	USING_WITH_STATIC_ASSERT(float_t, std::float_t, 1, 4)
		USING_WITH_STATIC_ASSERT(double_t, std::double_t, 1, 8)
		USING_WITH_STATIC_ASSERT(int32_t, std::int32_t, 1, 4)
		USING_WITH_STATIC_ASSERT(uint32_t, std::uint32_t, 1, 4)

		USING_WITH_STATIC_ASSERT(vec2, std::float_t, 2, 8)
		USING_WITH_STATIC_ASSERT(dvec2, std::double_t, 2, 16)
		USING_WITH_STATIC_ASSERT(ivec2, std::int32_t, 2, 8)
		USING_WITH_STATIC_ASSERT(uvec2, std::uint32_t, 2, 8)

		USING_WITH_STATIC_ASSERT(vec3, std::float_t, 3, 12)
		USING_WITH_STATIC_ASSERT(dvec3, std::double_t, 3, 24)
		USING_WITH_STATIC_ASSERT(ivec3, std::int32_t, 3, 12)
		USING_WITH_STATIC_ASSERT(uvec3, std::uint32_t, 3, 12)

		USING_WITH_STATIC_ASSERT(vec4, std::float_t, 4, 16)
		USING_WITH_STATIC_ASSERT(dvec4, std::double_t, 4, 32)
		USING_WITH_STATIC_ASSERT(ivec4, std::int32_t, 4, 16)
		USING_WITH_STATIC_ASSERT(uvec4, std::uint32_t, 4, 16)

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

		class struct_t;

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
		struct_t
		>;

	auto type_string(const arg_t& arg) -> const std::string;
	auto type_string(const struct_t& arg) -> const std::string;

	class struct_t {
	public:
		struct_t() {};
		struct_t(std::initializer_list<std::pair<std::string, refrakt::arg_t>> l);
		struct_t(const struct_t&) = default;
		auto add(const std::string& name, const std::string& type) -> refrakt::arg_t&;
		auto get(const std::string& name) -> refrakt::arg_t&;
		auto get(const std::string& name) const -> const refrakt::arg_t&;
		auto list() const ->std::vector<std::string>;
		auto type_string() const -> const std::string;

		template<typename T>
		auto get(const std::string& name) const -> const T& { return std::get<T>(get(name)); }

		template<typename T>
		auto get(const std::string& name) -> T& { return std::get<T>(get(name)); }

		auto begin() const { return members_.begin(); }
		auto end() const { return members_.end(); }

		bool has(const std::string& name) { return members_.count(name) > 0; }

		auto operator[](const std::string& key) -> refrakt::arg_t& { return members_[key]; }

	private:
		std::map<std::string, refrakt::arg_t> members_{};
	};

	void to_json(nlohmann::json& j, const refrakt::struct_t& s);
	void to_json(nlohmann::json& j, const refrakt::arg_t& a);
	void from_json(const nlohmann::json& j, refrakt::struct_t& s);

}
