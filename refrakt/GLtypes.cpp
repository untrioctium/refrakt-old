#include "GLtypes.hpp"
#include "type_helpers.hpp"
#include <initializer_list>
#include <iostream>

namespace /*type_string*/ refrakt {
#define MAKE_TYPE_STRING_FUNCTION(name) auto type_string( const name& value ) -> const std::string { return #name; }

	auto type_string(const arg_t& arg) -> const std::string {
		return std::visit([](auto&& v) -> const std::string {
			return type_string(v);
		}, arg);
	}

	auto type_string(const refrakt::float_t&) -> const std::string { return "float"; }
	auto type_string(const refrakt::double_t&) -> const std::string { return "double"; }
	auto type_string(const refrakt::int32_t&) -> const std::string { return "int32"; }
	auto type_string(const refrakt::uint32_t&) -> const std::string { return "uint32"; }
	auto type_string(const texture_handle&) -> const std::string { return "texture_handle"; }

	MAKE_TYPE_STRING_FUNCTION(vec2);
	MAKE_TYPE_STRING_FUNCTION(dvec2);
	MAKE_TYPE_STRING_FUNCTION(ivec2);
	MAKE_TYPE_STRING_FUNCTION(uvec2);

	MAKE_TYPE_STRING_FUNCTION(vec3);
	MAKE_TYPE_STRING_FUNCTION(dvec3);
	MAKE_TYPE_STRING_FUNCTION(ivec3);
	MAKE_TYPE_STRING_FUNCTION(uvec3);

	MAKE_TYPE_STRING_FUNCTION(vec4);
	MAKE_TYPE_STRING_FUNCTION(dvec4);
	MAKE_TYPE_STRING_FUNCTION(ivec4);
	MAKE_TYPE_STRING_FUNCTION(uvec4);

	MAKE_TYPE_STRING_FUNCTION(mat2x2);
	MAKE_TYPE_STRING_FUNCTION(mat2x3);
	MAKE_TYPE_STRING_FUNCTION(mat2x4);
	MAKE_TYPE_STRING_FUNCTION(mat3x2);
	MAKE_TYPE_STRING_FUNCTION(mat3x3);
	MAKE_TYPE_STRING_FUNCTION(mat3x4);
	MAKE_TYPE_STRING_FUNCTION(mat4x2);
	MAKE_TYPE_STRING_FUNCTION(mat4x3);
	MAKE_TYPE_STRING_FUNCTION(mat4x4);

	MAKE_TYPE_STRING_FUNCTION(dmat2x2);
	MAKE_TYPE_STRING_FUNCTION(dmat2x3);
	MAKE_TYPE_STRING_FUNCTION(dmat2x4);
	MAKE_TYPE_STRING_FUNCTION(dmat3x2);
	MAKE_TYPE_STRING_FUNCTION(dmat3x3);
	MAKE_TYPE_STRING_FUNCTION(dmat3x4);
	MAKE_TYPE_STRING_FUNCTION(dmat4x2);
	MAKE_TYPE_STRING_FUNCTION(dmat4x3);
	MAKE_TYPE_STRING_FUNCTION(dmat4x4);
}

namespace /*json*/ refrakt {

	void from_json(const nlohmann::json& j, refrakt::struct_t& s) {
		for (nlohmann::json::const_iterator it = j.cbegin(); it != j.cend(); ++it) {
			const std::size_t colon_location = it.key().find(":");
			std::string name = it.key().substr(0, colon_location);
			std::string type = it.key().substr(colon_location + 1);

			std::visit([&](auto&& arg) {
				using arg_type = std::decay_t<decltype(arg)>;

				if constexpr(refrakt::is_static_array_v<arg_type>) {
					const std::size_t size = std::min(arg.size(), it.value().size());
					for (std::size_t i = 0; i < size; i++)
						arg[i] = it.value()[i].get<arg_type::value_type>();
				}
			}, s.emplace(name, refrakt::type_helpers::factory(type)).first->second);
		}
	}
}