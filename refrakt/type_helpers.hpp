#pragma once

#include <GL/glew.h>
#include "GLtypes.hpp"
#include "imgui.h"

namespace refrakt::type_helpers::opengl {

	template< typename T >
	void push(GLuint handle, const std::string& name, const T& value) { static_assert(std::false_type::value, "unknown OpenGL type"); }

	void push(GLuint handle, const std::string& name, const refrakt::float_t& value);
	void push(GLuint handle, const std::string& name, const refrakt::double_t& value);
	void push(GLuint handle, const std::string& name, const refrakt::int32_t& value);
	void push(GLuint handle, const std::string& name, const refrakt::uint32_t& value);

	void push(GLuint handle, const std::string& name, const refrakt::vec2& value);
	void push(GLuint handle, const std::string& name, const refrakt::dvec2& value);
	void push(GLuint handle, const std::string& name, const refrakt::ivec2& value);
	void push(GLuint handle, const std::string& name, const refrakt::uvec2& value);

	void push(GLuint handle, const std::string& name, const refrakt::vec3& value);
	void push(GLuint handle, const std::string& name, const refrakt::dvec3& value);
	void push(GLuint handle, const std::string& name, const refrakt::ivec3& value);
	void push(GLuint handle, const std::string& name, const refrakt::uvec3& value);

	void push(GLuint handle, const std::string& name, const refrakt::vec4& value);
	void push(GLuint handle, const std::string& name, const refrakt::dvec4& value);
	void push(GLuint handle, const std::string& name, const refrakt::ivec4& value);
	void push(GLuint handle, const std::string& name, const refrakt::uvec4& value);

	void push(GLuint handle, const std::string& name, const refrakt::mat2x2& value);
	void push(GLuint handle, const std::string& name, const refrakt::mat2x3& value);
	void push(GLuint handle, const std::string& name, const refrakt::mat2x4& value);
	void push(GLuint handle, const std::string& name, const refrakt::mat3x2& value);
	void push(GLuint handle, const std::string& name, const refrakt::mat3x3& value);
	void push(GLuint handle, const std::string& name, const refrakt::mat3x4& value);
	void push(GLuint handle, const std::string& name, const refrakt::mat4x2& value);
	void push(GLuint handle, const std::string& name, const refrakt::mat4x3& value);
	void push(GLuint handle, const std::string& name, const refrakt::mat4x4& value);

	void push(GLuint handle, const std::string& name, const refrakt::dmat2x2& value);
	void push(GLuint handle, const std::string& name, const refrakt::dmat2x3& value);
	void push(GLuint handle, const std::string& name, const refrakt::dmat2x4& value);
	void push(GLuint handle, const std::string& name, const refrakt::dmat3x2& value);
	void push(GLuint handle, const std::string& name, const refrakt::dmat3x3& value);
	void push(GLuint handle, const std::string& name, const refrakt::dmat3x4& value);
	void push(GLuint handle, const std::string& name, const refrakt::dmat4x2& value);
	void push(GLuint handle, const std::string& name, const refrakt::dmat4x3& value);
	void push(GLuint handle, const std::string& name, const refrakt::dmat4x4& value);

	void push(GLuint handle, const std::string& name, const refrakt::arg_t& value);
}

namespace refrakt::type_helpers::imgui {

	template<typename T> constexpr ImGuiDataType scalar_flag() {
		if constexpr(std::is_same_v<T, std::float_t>) return ImGuiDataType_Float;
		if constexpr(std::is_same_v<T, std::double_t>) return ImGuiDataType_Double;
		if constexpr(std::is_same_v<T, std::uint32_t>) return ImGuiDataType_U32;
		if constexpr(std::is_same_v<T, std::int32_t>) return ImGuiDataType_S32;
	}

	template< typename T >
	bool display(T& value, const std::string& description, refrakt::dvec2 bounds, const float speed) { return false; } // TODO: struct_t and array_t handling 

	template< typename T, std::size_t s >
	bool display(refrakt::static_array_t<T, s>& value, const std::string& description, refrakt::dvec2 bounds, const float speed) {
		if constexpr(!std::is_arithmetic_v<T>) return false; // TODO: Properly deal with matrices (arrays of vecns)
		else {
			const T min_val = static_cast<T>(bounds[0]);
			const T max_val = static_cast<T>(bounds[1]);

			return ImGui::DragScalarN(description.c_str(), scalar_flag<T>(), value.data(), s, speed, &min_val, &max_val);
		}
	}

	bool display(refrakt::arg_t& value, const std::string& description, refrakt::dvec2 bounds, const float speed);
}

namespace refrakt::type_helpers {
	auto factory(const std::string& name) -> refrakt::arg_t;
	inline bool are_same_type(const refrakt::arg_t& a, const refrakt::arg_t& b) { return a.index() == b.index(); }
}