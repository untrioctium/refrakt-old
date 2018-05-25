#include "imgui.h"
#include "imgui-SFML.h"
#include "sol.hpp"
#include "json.hpp"

#include <iostream>
#include <string>
#include <fstream>

#include "RefraktWidget.hpp"

#include <experimental/filesystem>

#include "GLtypes.hpp"
#include "base64.h"

namespace fs = std::experimental::filesystem;
using json = nlohmann::json;

const std::set<std::string> RefraktWidget::simple_types_{ "int", "uint", "float", "double", "bool" };

std::shared_ptr<RefraktWidget> RefraktWidget::load(std::string program_name)
{
	fs::path package_file("programs");
	package_file /= program_name;

	auto ptr = std::make_shared<RefraktWidget>();

	ptr->lua_state_.open_libraries(sol::lib::base, sol::lib::io);
	ptr->loadBindings();

	// TODO: Error handling 
	sol::table defines = ptr->lua_state_.script_file(package_file.string());
	ptr->parameters_ = ptr->lua_state_.create_table();

	defines["parameters"].get<sol::table>().for_each([ptr](sol::object const& key, sol::object const& value) {
		auto def = value.as<sol::table>();
		std::string name = def["name"].get<std::string>();
		std::string type = def["type"].get<std::string>();

		ptr->draw_order_.push_back(name);
		ptr->parameters_[name] = def;

		if (def["init"].valid())
			ptr->parameters_[name]["value"] = def["init"];
		else if (ptr->registered_types_.count(type) == 1)
			ptr->parameters_[name]["value"] = ptr->getLuaState()[type]();
		else
			ptr->parameters_[name]["value"] = 0;

		std::cout << "Registered '" << name << "' (" << type << ")" << std::endl;
	});

	return ptr;
}

void RefraktWidget::drawGui()
{
	for (auto p : this->draw_order_) {
		std::string type = this->parameters_[p]["type"];
		if (this->registered_types_.count(type) == 1) {
			this->lua_state_[type]["meta"]["gui"](this->parameters_[p]["value"], this->parameters_[p]);
			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(this->parameters_[p]["tool_tip"].get<const char*>());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
		}
	}
}

std::string RefraktWidget::serialize() {
	nlohmann::json out;

	for (auto p : this->draw_order_) {
		sol::object result = this->parameters_[p]["value"]["serialize"](this->parameters_[p]["value"]);
		out.emplace(p, result.as<nlohmann::json>());
	}

	std::string result = out.dump();
	result = "RFKT" + base64_encode((const unsigned char*)result.c_str(), result.size());

	return result;
}

bool RefraktWidget::deserialize(std::string s) {
	if (s.substr(0, 4) != "RFKT") return false;

	auto result = nlohmann::json::parse(base64_decode(s.substr(4)));

	for (nlohmann::json::iterator it = result.begin(); it != result.end(); it++) {
		this->parameters_[it.key()]["value"]["deserialize"](this->parameters_[it.key()]["value"], it.value());
	}

	return true;
}

void RefraktWidget::loadBindings()
{

	auto imgui_arr_binder = [&state = this->lua_state_](std::string name, auto v, ImGuiDataType type, std::string default_format) {
		using arr_name = decltype(v);
		using arr_type = decltype(v.get<0>());

		state[name]["meta"]["gui"] = [default_format, type](arr_name& self, const sol::table& p) {
			arr_type min = p["bounds"][1].get<arr_type>();
			arr_type max = p["bounds"][2].get<arr_type>();

			return ImGui::DragScalarN(p["description"].get<const char*>(),
				type, &self, arr_name::len, p["speed"].get_or<float>(1.0), &min, &max,
				p["format"].get_or<std::string>(default_format).c_str(),
				p["power"].get_or<float>(1.0));
		};
	};

	auto base_array_bind = [&state = this->lua_state_, &type_set = this->registered_types_](std::string name, auto a, auto ...members) {
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
			"meta", sol::var(sol::reference(state.create_table())),
			"serialize", &arr_name::serialize,
			"deserialize", &arr_name::deserialize,
			members...);

		state[name]["meta"]["type"] = name;

		type_set.insert(name);
	};

	/* base type bindings */ {
		auto simple_bind = [&base_array_bind](std::string name, auto a) {
			using arr_name = decltype(a);

			base_array_bind(name, a,
				"v", sol::property(&arr_name::get<0>, &arr_name::set<0>)
			);
		};

		simple_bind("float", rfkt::float_t());
		imgui_arr_binder("float", rfkt::float_t(), ImGuiDataType_Float, "%.8f");

		simple_bind("double", rfkt::double_t());
		imgui_arr_binder("double", rfkt::double_t(), ImGuiDataType_Double, "%.8f");

		simple_bind("int32", rfkt::int32_t());
		imgui_arr_binder("int32", rfkt::int32_t(), ImGuiDataType_S32, "%d");

		simple_bind("uint32", rfkt::uint32_t());
		imgui_arr_binder("uint32", rfkt::uint32_t(), ImGuiDataType_U32, "%d");

		simple_bind("bool", rfkt::bool_t());
	}

	/* vec2 bindings */ {
		auto vec2_bind = [&base_array_bind](std::string name, auto v) {
			using vec_name = decltype(v);

			base_array_bind(name, v,
				"x", sol::property(&vec_name::get<0>, &vec_name::set<0>),
				"y", sol::property(&vec_name::get<1>, &vec_name::set<1>)
			);
		};

		vec2_bind("vec2", vec2());
		imgui_arr_binder("vec2", vec2(), ImGuiDataType_Float, "%.8f");

		vec2_bind("dvec2", dvec2());
		imgui_arr_binder("dvec2", dvec2(), ImGuiDataType_Double, "%.8f");

		vec2_bind("ivec2", ivec2());
		imgui_arr_binder("ivec2", ivec2(), ImGuiDataType_S32, "%d");

		vec2_bind("uvec2", uvec2());
		imgui_arr_binder("uvec2", uvec2(), ImGuiDataType_U32, "%d");

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
		imgui_arr_binder("vec3", vec3(), ImGuiDataType_Float, "%.8f");

		vec3_bind("dvec3", dvec3());
		imgui_arr_binder("dvec3", dvec3(), ImGuiDataType_Double, "%.8f");

		vec3_bind("ivec3", ivec3());
		imgui_arr_binder("ivec3", ivec3(), ImGuiDataType_S32, "%d");

		vec3_bind("uvec3", uvec3());
		imgui_arr_binder("uvec3", uvec3(), ImGuiDataType_U32, "%d");

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
		imgui_arr_binder("vec4", vec4(), ImGuiDataType_Float, "%.8f");

		vec4_bind("dvec4", dvec4());
		imgui_arr_binder("dvec4", dvec4(), ImGuiDataType_Double, "%.8f");

		vec4_bind("ivec4", ivec4());
		imgui_arr_binder("ivec4", ivec4(), ImGuiDataType_S32, "%d");

		vec4_bind("uvec4", uvec4());
		imgui_arr_binder("uvec4", uvec4(), ImGuiDataType_U32, "%d");

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
