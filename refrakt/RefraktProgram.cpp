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

	sol::table defines = ptr->lua_state.script_file(package_file.string());

	defines["parameters"].get<sol::table>().for_each([](sol::object const& key, sol::object const& value) {
		value.get_type();
	});

	return ptr;
}

void RefraktProgram::loadBindings(sol::state& state)
{

	/* vec2 bindings */ {
		auto bind = [&state](std::string name, auto v) {
			using v_name = decltype(v);
			using v_type = decltype(v_name::x);

			state.new_usertype<v_name>(name,
				sol::call_constructor, sol::constructors<v_name(), v_name(v_type), v_name(v_type, v_type)>(),
				"x", &v_name::x,
				"y", &v_name::y
				);
		};

		bind("vec2", vec2());
		state["vec2"]["imgui"] = [](vec2& self, const sol::table& p) {
			return ImGui::SliderFloat2(p["name"].get<const char*>(), 
				&self.x,
				p["bounds"]["min"], 
				p["bounds"]["max"],
				p["format"].get_or<std::string>("%.3f").c_str(),
				p["power"].get_or(1.0f)
			);
		};

		bind("dvec2", dvec2() );
		bind("ivec2", ivec2() );
		bind("uvec2", uvec2() );
		bind("bvec2", bvec2() );
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
