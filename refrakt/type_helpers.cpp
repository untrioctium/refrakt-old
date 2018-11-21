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