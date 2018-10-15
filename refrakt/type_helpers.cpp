#include "type_helpers.hpp"

namespace refrakt::type_helpers::opengl {

	void push(GLuint handle, const std::string& name, const refrakt::float_t& value) { glUniform1f(glGetUniformLocation(handle, name.c_str()), value[0]); }
	void push(GLuint handle, const std::string& name, const refrakt::double_t& value) { glUniform1d(glGetUniformLocation(handle, name.c_str()), value[0]); }
	void push(GLuint handle, const std::string& name, const refrakt::int32_t& value) { glUniform1i(glGetUniformLocation(handle, name.c_str()), value[0]); }
	void push(GLuint handle, const std::string& name, const refrakt::uint32_t& value) { glUniform1ui(glGetUniformLocation(handle, name.c_str()), value[0]); }

	void push(GLuint handle, const std::string& name, const refrakt::vec2& value) { glUniform2fv(glGetUniformLocation(handle, name.c_str()), 1, value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::dvec2& value) { glUniform2dv(glGetUniformLocation(handle, name.c_str()), 1, value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::ivec2& value) { glUniform2iv(glGetUniformLocation(handle, name.c_str()), 1, value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::uvec2& value) { glUniform2uiv(glGetUniformLocation(handle, name.c_str()), 1, value.data()); }

	void push(GLuint handle, const std::string& name, const refrakt::vec3& value) { glUniform3fv(glGetUniformLocation(handle, name.c_str()), 1, value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::dvec3& value) { glUniform3dv(glGetUniformLocation(handle, name.c_str()), 1, value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::ivec3& value) { glUniform3iv(glGetUniformLocation(handle, name.c_str()), 1, value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::uvec3& value) { glUniform3uiv(glGetUniformLocation(handle, name.c_str()), 1, value.data()); }

	void push(GLuint handle, const std::string& name, const refrakt::vec4& value) { glUniform4fv(glGetUniformLocation(handle, name.c_str()), 1, value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::dvec4& value) { glUniform4dv(glGetUniformLocation(handle, name.c_str()), 1, value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::ivec4& value) { glUniform4iv(glGetUniformLocation(handle, name.c_str()), 1, value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::uvec4& value) { glUniform4uiv(glGetUniformLocation(handle, name.c_str()), 1, value.data()); }

	void push(GLuint handle, const std::string& name, const refrakt::mat2x2& value) { glUniformMatrix2fv(glGetUniformLocation(handle, name.c_str()), 1, false, (float*)value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::mat2x3& value) { glUniformMatrix2x3fv(glGetUniformLocation(handle, name.c_str()), 1, false, (float*)value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::mat2x4& value) { glUniformMatrix2x4fv(glGetUniformLocation(handle, name.c_str()), 1, false, (float*)value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::mat3x2& value) { glUniformMatrix3x2fv(glGetUniformLocation(handle, name.c_str()), 1, false, (float*)value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::mat3x3& value) { glUniformMatrix3fv(glGetUniformLocation(handle, name.c_str()), 1, false, (float*)value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::mat3x4& value) { glUniformMatrix3x4fv(glGetUniformLocation(handle, name.c_str()), 1, false, (float*)value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::mat4x2& value) { glUniformMatrix4x2fv(glGetUniformLocation(handle, name.c_str()), 1, false, (float*)value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::mat4x3& value) { glUniformMatrix4x3fv(glGetUniformLocation(handle, name.c_str()), 1, false, (float*)value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::mat4x4& value) { glUniformMatrix4fv(glGetUniformLocation(handle, name.c_str()), 1, false, (float*)value.data()); }

	void push(GLuint handle, const std::string& name, const refrakt::dmat2x2& value) { glUniformMatrix2dv(glGetUniformLocation(handle, name.c_str()), 1, false, (double*)value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::dmat2x3& value) { glUniformMatrix2x3dv(glGetUniformLocation(handle, name.c_str()), 1, false, (double*)value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::dmat2x4& value) { glUniformMatrix2x4dv(glGetUniformLocation(handle, name.c_str()), 1, false, (double*)value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::dmat3x2& value) { glUniformMatrix3x2dv(glGetUniformLocation(handle, name.c_str()), 1, false, (double*)value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::dmat3x3& value) { glUniformMatrix3dv(glGetUniformLocation(handle, name.c_str()), 1, false, (double*)value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::dmat3x4& value) { glUniformMatrix3x4dv(glGetUniformLocation(handle, name.c_str()), 1, false, (double*)value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::dmat4x2& value) { glUniformMatrix4x2dv(glGetUniformLocation(handle, name.c_str()), 1, false, (double*)value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::dmat4x3& value) { glUniformMatrix4x3dv(glGetUniformLocation(handle, name.c_str()), 1, false, (double*)value.data()); }
	void push(GLuint handle, const std::string& name, const refrakt::dmat4x4& value) { glUniformMatrix4dv(glGetUniformLocation(handle, name.c_str()), 1, false, (double*)value.data()); }

	void push(GLuint handle, const std::string& name, const refrakt::struct_t& value) {
		for (auto&& mem : value)
			push(handle, name + "." + mem.first, mem.second);
	}

	void push(GLuint handle, const std::string& name, const refrakt::arg_t& value) { std::visit([handle, &name](auto&& value) { 
		if constexpr(is_static_array_v<std::decay_t<decltype(value)>>)
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