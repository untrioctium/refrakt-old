#pragma once

#include <GL/glew.h>
#include <imgui.h>

#include "GLtypes.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace refrakt::type_helpers::opengl {
	
	namespace detail {
		template<typename T>
		const auto pusher = std::false_type;

		template<> const auto pusher<refrakt::float_t> = &glUniform1fv;
		template<> const auto pusher<refrakt::double_t> = &glUniform1dv;
		template<> const auto pusher<refrakt::int32_t> = &glUniform1iv;
		template<> const auto pusher<refrakt::uint32_t> = &glUniform1uiv;
		template<> const auto pusher<refrakt::vec2> = &glUniform2fv;
		template<> const auto pusher<refrakt::dvec2> = &glUniform2dv;
		template<> const auto pusher<refrakt::ivec2> = &glUniform2iv;
		template<> const auto pusher<refrakt::uvec2> = &glUniform2uiv;
		template<> const auto pusher<refrakt::vec3> = &glUniform3fv;
		template<> const auto pusher<refrakt::dvec3> = &glUniform3dv;
		template<> const auto pusher<refrakt::ivec3> = &glUniform3iv;
		template<> const auto pusher<refrakt::uvec3> = &glUniform3uiv;
		template<> const auto pusher<refrakt::vec4> = &glUniform4fv;
		template<> const auto pusher<refrakt::dvec4> = &glUniform4dv;
		template<> const auto pusher<refrakt::ivec4> = &glUniform4iv;
		template<> const auto pusher<refrakt::uvec4> = &glUniform4uiv;
		template<> const auto pusher<refrakt::mat2x2> = &glUniformMatrix2fv;
		template<> const auto pusher<refrakt::mat2x3> = &glUniformMatrix2x3fv;
		template<> const auto pusher<refrakt::mat2x4> = &glUniformMatrix2x4fv;
		template<> const auto pusher<refrakt::mat3x2> = &glUniformMatrix3x2fv;
		template<> const auto pusher<refrakt::mat3x3> = &glUniformMatrix3fv;
		template<> const auto pusher<refrakt::mat3x4> = &glUniformMatrix3x4fv;
		template<> const auto pusher<refrakt::mat4x2> = &glUniformMatrix4x2fv;
		template<> const auto pusher<refrakt::mat4x3> = &glUniformMatrix4x3fv;
		template<> const auto pusher<refrakt::mat4x4> = &glUniformMatrix4fv;
		template<> const auto pusher<refrakt::dmat2x2> = &glUniformMatrix2dv;
		template<> const auto pusher<refrakt::dmat2x3> = &glUniformMatrix2x3dv;
		template<> const auto pusher<refrakt::dmat2x4> = &glUniformMatrix2x4dv;
		template<> const auto pusher<refrakt::dmat3x2> = &glUniformMatrix3x2dv;
		template<> const auto pusher<refrakt::dmat3x3> = &glUniformMatrix3dv;
		template<> const auto pusher<refrakt::dmat3x4> = &glUniformMatrix3x4dv;
		template<> const auto pusher<refrakt::dmat4x2> = &glUniformMatrix4x2dv;
		template<> const auto pusher<refrakt::dmat4x3> = &glUniformMatrix4x3dv;
		template<> const auto pusher<refrakt::dmat4x4> = &glUniformMatrix4dv;
	}

	void push(GLuint handle, const std::string& name, const refrakt::arg_t& value);

	template<typename Type>
	void push(GLuint handle, const std::string& name, const Type& value) {
		auto location = glGetUniformLocation(handle, name.c_str());

		if constexpr (is_vector_type<Type>) 
			(*detail::pusher<Type>)(location, 1, (typename Type::value_type*) &value);
		else if constexpr (is_matrix_type<Type>) 
			(*detail::pusher<Type>)(location, 1, false, (typename Type::value_type*) &value);
		else if constexpr (is_array_type<Type>) {
			auto size_location = glGetUniformLocation(handle, (name + "_size").c_str());
			auto size = value.max_size();
			if constexpr(is_vector_type<Type::value_type>)
				(*detail::pusher<Type>)(location, size, (typename Type::value_type::value_type*) value.data());
			else if constexpr (is_matrix_type<Type::value_type>)
				(*detail::pusher<Type>)(location, size, false, (typename Type::value_type::value_type*) value.data());

			glUniform1ui(size_location, value.size());
		}
	}
}

namespace refrakt::type_helpers::imgui {

	template<typename T> constexpr ImGuiDataType scalar_flag() {
		if constexpr(std::is_same_v<T, std::float_t>) return ImGuiDataType_Float;
		if constexpr(std::is_same_v<T, std::double_t>) return ImGuiDataType_Double;
		if constexpr(std::is_same_v<T, std::uint32_t>) return ImGuiDataType_U32;
		if constexpr(std::is_same_v<T, std::int32_t>) return ImGuiDataType_S32;
	}

	template< typename T >
	bool display(T& value, const std::string& description, refrakt::dvec2 bounds, const float speed) { return false; }

	template< std::size_t S, typename T, glm::qualifier Q >
	bool display(glm::vec<S,T,Q> & value, const std::string& description, refrakt::dvec2 bounds, const float speed) {
		if constexpr(!std::is_arithmetic_v<T>) return false;
		else {
			const T min_val = static_cast<T>(bounds[0]);
			const T max_val = static_cast<T>(bounds[1]);

			return ImGui::DragScalarN(description.c_str(), scalar_flag<T>(), &value[0], S, speed, &min_val, &max_val);
		}
	}

	bool display(refrakt::arg_t& value, const std::string& description, refrakt::dvec2 bounds, const float speed);
}

namespace refrakt::type_helpers {
	auto factory(const std::string& name) -> refrakt::arg_t;
	inline bool are_same_type(const refrakt::arg_t& a, const refrakt::arg_t& b) { return a.index() == b.index(); }

	template<typename T>
	T mix(const T& a, const T& b, typename T::value_type v) {
		return a * ( typename T::value_type(1.0) - v )  + b * v;
	}
}