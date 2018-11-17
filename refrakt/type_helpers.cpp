#include "type_helpers.hpp"

namespace refrakt::type_helpers::opengl {

	void push(GLuint handle, const std::string& name, const refrakt::struct_t& value) {
		for (auto&& mem : value)
			push(handle, name + "." + mem.first, mem.second);
	}

	void push(GLuint handle, const std::string& name, const refrakt::arg_t& value) { std::visit([handle, &name](auto&& value) { 
		if constexpr(is_vector_type<std::decay_t<decltype(value)>>)
			push(handle, name, value); 
	}, value); }

}

bool refrakt::type_helpers::imgui::display(refrakt::arg_t& value, const std::string& description, refrakt::dvec2 bounds, const float speed) {
	return std::visit(
		[&] (auto&& v) { return display(v, description, bounds, speed); }, 
		value
	);
}

refrakt::arg_t refrakt::type_helpers::factory(const std::string& name) {
#define MAKE_TYPE_FACTORY(name) {#name, refrakt::name{}}

	static std::map<std::string, refrakt::arg_t> types {
		{"float", refrakt::float_t{}},
		{ "double", refrakt::double_t{} },
		{ "int32", refrakt::int32_t{} },
		{ "uint32", refrakt::uint32_t{} },
		MAKE_TYPE_FACTORY(vec2),
		MAKE_TYPE_FACTORY(dvec2),
		MAKE_TYPE_FACTORY(ivec2),
		MAKE_TYPE_FACTORY(uvec2),
		MAKE_TYPE_FACTORY(vec3),
		MAKE_TYPE_FACTORY(dvec3),
		MAKE_TYPE_FACTORY(ivec3),
		MAKE_TYPE_FACTORY(uvec3),
		MAKE_TYPE_FACTORY(vec4),
		MAKE_TYPE_FACTORY(dvec4),
		MAKE_TYPE_FACTORY(ivec4),
		MAKE_TYPE_FACTORY(uvec4),
		MAKE_TYPE_FACTORY(mat2x2),
		MAKE_TYPE_FACTORY(mat2x3),
		MAKE_TYPE_FACTORY(mat2x4),
		MAKE_TYPE_FACTORY(mat3x2),
		MAKE_TYPE_FACTORY(mat3x3),
		MAKE_TYPE_FACTORY(mat3x4),
		MAKE_TYPE_FACTORY(mat3x2),
		MAKE_TYPE_FACTORY(mat3x3),
		MAKE_TYPE_FACTORY(mat3x4),
		MAKE_TYPE_FACTORY(mat4x2),
		MAKE_TYPE_FACTORY(mat4x3),
		MAKE_TYPE_FACTORY(mat4x4),
		MAKE_TYPE_FACTORY(dmat2x2),
		MAKE_TYPE_FACTORY(dmat2x3),
		MAKE_TYPE_FACTORY(dmat2x4),
		MAKE_TYPE_FACTORY(dmat3x2),
		MAKE_TYPE_FACTORY(dmat3x3),
		MAKE_TYPE_FACTORY(dmat3x4),
		MAKE_TYPE_FACTORY(dmat3x2),
		MAKE_TYPE_FACTORY(dmat3x3),
		MAKE_TYPE_FACTORY(dmat3x4),
		MAKE_TYPE_FACTORY(dmat4x2),
		MAKE_TYPE_FACTORY(dmat4x3),
		MAKE_TYPE_FACTORY(dmat4x4),
	};

	return types.find(name)->second;
}