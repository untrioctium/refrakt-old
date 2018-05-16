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

	auto base_vec_bind = [&state](std::string name, auto v, auto ...members) {
		using vec_name = decltype(v);
		using vec_type = decltype(vec_name::x);

		state.new_usertype<vec_name>(name,
			sol::call_constructor, sol::constructors<vec_name(), vec_name(vec_type), vec_name::unique_constructor>(),
			members...);
	};


	/* vec2 bindings */ {
		// Generic bind function for any 2-element vector
		// v is only used to deduce types
		auto vec2_bind = [&base_vec_bind](std::string name, auto v) {
			using vec_name = decltype(v);

			base_vec_bind(name, v,
				"x", &vec_name::x,
				"y", &vec_name::y);
		};

		vec2_bind("vec2", vec2());
		imgui_vec_binder("vec2", vec2(), ImGui::SliderFloat2, "%.3f");

		vec2_bind("dvec2", dvec2() );
		vec2_bind("ivec2", ivec2() );
		//imgui_vec_binder("vec2", vec2(), ImGui::SliderInt2, "%d");

		vec2_bind("uvec2", uvec2() );
		vec2_bind("bvec2", bvec2() );
	}
	/* vec3 bindings */ {
		auto bind = [&state](std::string name, auto v) {
			using v_name = decltype(v);
			using v_type = decltype(v_name::x);

			state.new_usertype<v_name>(name,
				sol::call_constructor, sol::constructors<v_name(), v_name(v_type), v_name(v_type, v_type, v_type)>(),
				"x", &v_name::x,
				"y", &v_name::y,
				"z", &v_name::z
				);
		};

		bind("vec3", vec3());
		imgui_vec_binder("vec3", vec3(), ImGui::SliderFloat3, "%.3f");
		bind("dvec3", dvec3());
		bind("ivec3", ivec3());
		bind("uvec3", uvec3());
		bind("bvec3", bvec3());
	}
	/* vec4 bindings */ {
		auto bind = [&state](std::string name, auto v) {
			using v_name = decltype(v);
			using v_type = decltype(v_name::x);

			state.new_usertype<v_name>(name,
				sol::call_constructor, sol::constructors<v_name(), v_name(v_type), v_name(v_type, v_type, v_type, v_type)>(),
				"x", &v_name::x,
				"y", &v_name::y,
				"z", &v_name::z,
				"w", &v_name::w
				);
		};

		bind("vec4", vec4());
		bind("dvec4", dvec4());
		bind("ivec4", ivec4());
		bind("uvec4", uvec4());
		bind("bvec4", bvec4());
	}
}
