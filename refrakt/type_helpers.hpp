#pragma once

#include <GL/glew.h>
#include "GLtypes.hpp"

namespace refrakt::opengl::type_helpers {

	template< typename T >
	void push(GLuint id, const T& value) { static_assert(std::false_type::value, "unknown OpenGL type"); }

	void push(GLuint id, const refrakt::float_t& value) { glUniform1f(id, value[0]); }
	void push(GLuint id, const refrakt::double_t& value) { glUniform1d(id, value[0]); }
	void push(GLuint id, const refrakt::int32_t& value) { glUniform1i(id, value[0]); }
	void push(GLuint id, const refrakt::uint32_t& value) { glUniform1ui(id, value[0]); }

	void push(GLuint id, const refrakt::vec2& value) { glUniform2fv(id, 1, value.data()); }
	void push(GLuint id, const refrakt::dvec2& value) { glUniform2dv(id, 1, value.data()); }
	void push(GLuint id, const refrakt::ivec2& value) { glUniform2iv(id, 1, value.data()); }
	void push(GLuint id, const refrakt::uvec2& value) { glUniform2uiv(id, 1, value.data()); }

	void push(GLuint id, const refrakt::vec3& value) { glUniform3fv(id, 1, value.data()); }
	void push(GLuint id, const refrakt::dvec3& value) { glUniform3dv(id, 1, value.data()); }
	void push(GLuint id, const refrakt::ivec3& value) { glUniform3iv(id, 1, value.data()); }
	void push(GLuint id, const refrakt::uvec3& value) { glUniform3uiv(id, 1, value.data()); }

	void push(GLuint id, const refrakt::vec4& value) { glUniform4fv(id, 1, value.data()); }
	void push(GLuint id, const refrakt::dvec4& value) { glUniform4dv(id, 1, value.data()); }
	void push(GLuint id, const refrakt::ivec4& value) { glUniform4iv(id, 1, value.data()); }
	void push(GLuint id, const refrakt::uvec4& value) { glUniform4uiv(id, 1, value.data()); }

	void push(GLuint id, const refrakt::mat2x2& value) { glUniformMatrix2fv(id, 1, false, (float*) value.data()); }
	void push(GLuint id, const refrakt::mat2x3& value) { glUniformMatrix2x3fv(id, 1, false, (float*)value.data()); }
	void push(GLuint id, const refrakt::mat2x4& value) { glUniformMatrix2x4fv(id, 1, false, (float*)value.data()); }
	void push(GLuint id, const refrakt::mat3x2& value) { glUniformMatrix3x2fv(id, 1, false, (float*)value.data()); }
	void push(GLuint id, const refrakt::mat3x3& value) { glUniformMatrix3fv(id, 1, false, (float*)value.data()); }
	void push(GLuint id, const refrakt::mat3x4& value) { glUniformMatrix3x4fv(id, 1, false, (float*)value.data()); }
	void push(GLuint id, const refrakt::mat4x2& value) { glUniformMatrix4x2fv(id, 1, false, (float*)value.data()); }
	void push(GLuint id, const refrakt::mat4x3& value) { glUniformMatrix4x3fv(id, 1, false, (float*)value.data()); }
	void push(GLuint id, const refrakt::mat4x4& value) { glUniformMatrix4fv(id, 1, false, (float*)value.data()); }

	void push(GLuint id, const refrakt::dmat2x2& value) { glUniformMatrix2dv(id, 1, false, (double*)value.data()); }
	void push(GLuint id, const refrakt::dmat2x3& value) { glUniformMatrix2x3dv(id, 1, false, (double*)value.data()); }
	void push(GLuint id, const refrakt::dmat2x4& value) { glUniformMatrix2x4dv(id, 1, false, (double*)value.data()); }
	void push(GLuint id, const refrakt::dmat3x2& value) { glUniformMatrix3x2dv(id, 1, false, (double*)value.data()); }
	void push(GLuint id, const refrakt::dmat3x3& value) { glUniformMatrix3dv(id, 1, false, (double*)value.data()); }
	void push(GLuint id, const refrakt::dmat3x4& value) { glUniformMatrix3x4dv(id, 1, false, (double*)value.data()); }
	void push(GLuint id, const refrakt::dmat4x2& value) { glUniformMatrix4x2dv(id, 1, false, (double*)value.data()); }
	void push(GLuint id, const refrakt::dmat4x3& value) { glUniformMatrix4x3dv(id, 1, false, (double*)value.data()); }
	void push(GLuint id, const refrakt::dmat4x4& value) { glUniformMatrix4dv(id, 1, false, (double*)value.data()); }

	void push(GLuint id, const refrakt::arg_t& value) { std::visit([id](auto&& value) { push(id, value); }, value); }
}