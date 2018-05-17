#include "imgui.h"
#include "imgui-SFML.h"
#include "sol.hpp"
#include "json.hpp"

#include <iostream>
#include <string>
#include <fstream>

#include "RefraktProgram.hpp"

#include <experimental/filesystem>

#include "GLtypes.hpp"

namespace fs = std::experimental::filesystem;
using json = nlohmann::json;

std::shared_ptr<RefraktProgram> RefraktProgram::load(std::string program_name)
{
	fs::path package_file("programs");
	package_file /= program_name;

	auto ptr = std::make_shared<RefraktProgram>();
	ptr->lua_state.open_libraries(sol::lib::base, sol::lib::io);
	RefraktProgram::loadBindings(ptr->lua_state);

	// TODO: Error handling 
	sol::table defines = ptr->lua_state.script_file(package_file.string());

	defines["parameters"].get<sol::table>().for_each([](sol::object const& key, sol::object const& value) {
		value.get_type();
	});

	//ivec2 test(4, 5);
	//test[0] = 3;
	//std::cout << test[0] << std::endl;

	return ptr;
}

void RefraktProgram::loadBindings(sol::state& state)
{
	auto imgui_vec_binder = [&state](auto name, auto v, auto imgui_function, std::string default_format) {
		using vec_type = decltype(v);
		using vec_elem_type = decltype(vec_type::x);

		state[name]["gui"] = [imgui_function, default_format](vec_type& self, const sol::table& p) {
			auto ret = imgui_function(p["name"].get<const char*>(),
				&self.x, p["bounds"]["min"], p["bounds"]["max"],
				p["format"].get_or<std::string>(default_format).c_str(),
				p["power"].get_or((vec_elem_type)1));

			if (p["tool_tip"].valid()) {
				ImGui::SameLine();
				ImGui::TextDisabled("(?)");
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
					ImGui::TextUnformatted(p["tool_tip"].get<std::string>().c_str());
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}
			}

			return ret;
		};
	};

	auto base_array_bind = [&state](std::string name, auto a, auto ...members) {
		using arr_name = decltype(a);
		using arr_type = decltype(a.get<0>());

		auto factories = sol::factories(
			[]() { return arr_name(); }, // empty factory
			[](arr_type init) { return arr_name(init); }, // single initializer
			[](sol::variadic_args va) {
				if (va.size() != arr_name::len) throw sol::error("error creating array");

				auto result = arr_name();
				for (std::size_t i = 0; i < arr_name::len; i++) result[i] = va[i].get<arr_type>();
				return result;
			}
		);

		state.new_usertype<arr_name>(name,
			sol::call_constructor, factories,
			sol::meta_function::index, [](arr_name& a, std::size_t index) -> decltype(a[index]) { return a[index]; },
			sol::meta_function::new_index, [](arr_name& a, std::size_t index, arr_type v) { a[index] = v; },
			members...);
	};
	/* vec2 bindings */{
		auto vec2_bind = [&base_array_bind](std::string name, auto v) {
			using vec_name = decltype(v);

			base_array_bind(name, v,
				"x", sol::property(&vec_name::get<0>, &vec_name::set<0>),
				"y", sol::property(&vec_name::get<1>, &vec_name::set<1>)
			);
		};

		vec2_bind("vec2", vec2());
		vec2_bind("dvec2", dvec2());
		vec2_bind("ivec2", ivec2());
		vec2_bind("uvec2", uvec2());
		vec2_bind("bvec2", bvec2());
	}

	/* vec3 bindings */ {
		auto vec3_bind = [&base_array_bind](std::string name, auto v) {
			using vec_name = decltype(v);

			base_array_bind(name, v,
				"x", sol::property(&vec_name::get<0>, &vec_name::set<0>),
				"y", sol::property(&vec_name::get<1>, &vec_name::set<1>),
				"z", sol::property(&vec_name::get<2>, &vec_name::set<2>)
			);
		};

		vec3_bind("vec3", vec3());
		vec3_bind("dvec3", dvec3());
		vec3_bind("ivec3", ivec3());
		vec3_bind("uvec3", uvec3());
		vec3_bind("bvec3", bvec3());
	}

	/* vec4 bindings */ {
		auto vec4_bind = [&base_array_bind](std::string name, auto v) {
			using vec_name = decltype(v);

			base_array_bind(name, v,
				"x", sol::property(&vec_name::get<0>, &vec_name::set<0>),
				"y", sol::property(&vec_name::get<1>, &vec_name::set<1>),
				"z", sol::property(&vec_name::get<2>, &vec_name::set<2>),
				"w", sol::property(&vec_name::get<3>, &vec_name::set<3>)
			);
		};

		vec4_bind("vec4", vec4());
		vec4_bind("dvec4", dvec4());
		vec4_bind("ivec4", ivec4());
		vec4_bind("uvec4", uvec4());
		vec4_bind("bvec4", bvec4());
	}
	
	/* matrix bindings */ {
		base_array_bind("mat2x2", mat2x2());
		base_array_bind("mat2x3", mat2x3());
		base_array_bind("mat2x4", mat2x4());

		base_array_bind("mat3x2", mat3x2());
		base_array_bind("mat3x3", mat3x3());
		base_array_bind("mat3x4", mat3x4());

		base_array_bind("mat4x2", mat4x2());
		base_array_bind("mat4x3", mat4x3());
		base_array_bind("mat4x4", mat4x4());

		base_array_bind("dmat2x2", dmat2x2());
		base_array_bind("dmat2x3", dmat2x3());
		base_array_bind("dmat2x4", dmat2x4());

		base_array_bind("dmat3x2", dmat3x2());
		base_array_bind("dmat3x3", dmat3x3());
		base_array_bind("dmat3x4", dmat3x4());

		base_array_bind("dmat4x2", dmat4x2());
		base_array_bind("dmat4x3", dmat4x3());
		base_array_bind("dmat4x4", dmat4x4());
	}
}
