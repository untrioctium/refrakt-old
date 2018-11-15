#include "lua_modules.hpp"
#include "GLtypes.hpp"
#include "type_helpers.hpp"

struct mod_refrakt : refrakt::lua::modules::registrar<mod_refrakt> {
	mod_refrakt() {}

	static inline std::string name = "refrakt";
	static inline std::string parent = "global";
	static inline std::string description = "Refrakt types and functions";

	static void apply(sol::table mod) {
		apply_vec<refrakt::float_t>(mod);
		apply_vec<refrakt::double_t>(mod);
		apply_vec<refrakt::int32_t>(mod);
		apply_vec<refrakt::uint32_t>(mod);

		apply_vec<refrakt::vec2>(mod);
		apply_vec<refrakt::dvec2>(mod);
		apply_vec<refrakt::ivec2>(mod);
		apply_vec<refrakt::uvec2>(mod);

		apply_vec<refrakt::vec3>(mod);
		apply_vec<refrakt::dvec3>(mod);
		apply_vec<refrakt::ivec3>(mod);
		apply_vec<refrakt::uvec3>(mod);

		apply_vec<refrakt::vec4>(mod);
		apply_vec<refrakt::dvec4>(mod);
		apply_vec<refrakt::ivec4>(mod);
		apply_vec<refrakt::uvec4>(mod);
		
		mod.new_usertype<refrakt::texture>("texture",
			"type", sol::property([]() {return "texture"; }),
			"handle", sol::property(&refrakt::texture::handle),
			"w", sol::property([](const refrakt::texture& t) {return t.info().w; }),
			"h", sol::property([](const refrakt::texture& t) {return t.info().h; })
		);

		mod["mix"] = sol::overload(
			refrakt::type_helpers::mix<refrakt::float_t>,
			refrakt::type_helpers::mix<refrakt::double_t>,
			refrakt::type_helpers::mix<refrakt::vec2>,
			refrakt::type_helpers::mix<refrakt::dvec2>,
			refrakt::type_helpers::mix<refrakt::vec3>,
			refrakt::type_helpers::mix<refrakt::dvec3>,
			refrakt::type_helpers::mix<refrakt::vec4>,
			refrakt::type_helpers::mix<refrakt::dvec4>
		);

		mod["smooth_step"] = [](double x) -> double {
			if (x <= 0.0) return 0.0;
			if (x >= 1.0) return 1.0;
			return -2.0 * x * x * x + 3.0 * x * x;
		};

		mod["smoother_step"] = [](double x) -> double {
			if (x <= 0.0) return 0.0;
			if (x >= 1.0) return 1.0;
			return 6.0 * std::pow(x, 5) - 15.0 * std::pow(x, 4) + 10.0 * std::pow(x, 3);
		};

		mod["smoothest_step"] = [](double x) -> double {
			if (x <= 0.0) return 0.0;
			if (x >= 1.0) return 1.0;
			return 924.0 * std::pow(x, 13)
				- 6006.0 * std::pow(x, 12)
				+ 16380.0 * std::pow(x, 11)
				- 24024.0 * std::pow(x, 10)
				+ 20020.0 * std::pow(x, 9)
				- 9009.0 * std::pow(x, 8)
				+ 1716.0 * std::pow(x, 7);
		};
	}

	template<typename Base, typename Constructor, typename... Ts>
	static auto base_type_apply(Ts... args) {
		std::string name = refrakt::type_string(Base{});
		return [=](sol::table to) {
			to.new_usertype<Base>(name,
				sol::call_constructor, Constructor(),
				"type", sol::property([name]() {return name; }),
				args...);
		};
	}

	template<typename vec>
	static void apply_vec(sol::table mod) {
		if constexpr (vec::length() == 1) {
			base_type_apply<vec,
				sol::constructors<
				vec(),
				vec(vec::value_type)>
			>("x", &vec::x)(mod);
		}
		else if constexpr (vec::length() == 2) {
			base_type_apply<vec,
				sol::constructors<
				vec(),
				vec(vec::value_type),
				vec(vec::value_type, vec::value_type)>
			>("x", &vec::x, "y", &vec::y)(mod);
		}
		else if constexpr (vec::length() == 3) {
			base_type_apply<vec,
				sol::constructors<
				vec(),
				vec(vec::value_type),
				vec(vec::value_type, vec::value_type, vec::value_type)>
			>("x", &vec::x, "y", &vec::y, "z", &vec::z)(mod);
		}
		else if constexpr (vec::length() == 4) {
			base_type_apply<vec,
				sol::constructors<
				vec(),
				vec(vec::value_type),
				vec(vec::value_type, vec::value_type, vec::value_type, vec::value_type)>
			>("x", &vec::x, "y", &vec::y, "z", &vec::z, "w", &vec::w)(mod);
		}
	}
};