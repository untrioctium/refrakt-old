#include "lua_modules.hpp"

struct mod_gl_base : refrakt::lua::modules::registrar<mod_gl_base> {
	mod_gl_base() {}

	static inline std::string name = "base";
	static inline std::string parent = "gl";
	static inline std::string description = "Low-Level OpenGL functions and constants.";

	static void apply(sol::table mod) {
		// no-op for now
	}
};