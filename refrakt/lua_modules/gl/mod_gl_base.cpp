#include <gl/glew.h>
#include "lua_modules.hpp"
#include "texture.hpp"

struct mod_gl_base : refrakt::lua::modules::registrar<mod_gl_base> {
	mod_gl_base() {}

	static inline std::string name = "base";
	static inline std::string parent = "gl";
	static inline std::string description = "Low-Level OpenGL functions and constants.";

	static void apply(sol::table mod) {
		mod["viewport"] = glViewport;
		mod["draw_arrays"] = glDrawArrays;

		mod["TRIANGLE_STRIP"] = GL_TRIANGLE_STRIP;

		mod["active_texture"] = glActiveTexture;
		mod["bind_texture"] = [](GLenum target, refrakt::texture tex) {
			glBindTexture(target, tex.handle());
		};

		mod["TEXTURE_2D"] = GL_TEXTURE_2D;

	}
};