#include <GL/glew.h>
#include <spdlog/spdlog.h>

#include <iostream>
#include "GLtypes.hpp"
#include "type_helpers.hpp"
#include "lua_modules.hpp"

struct mod_gl_program : refrakt::lua::modules::registrar<mod_gl_program> {
	mod_gl_program() {}

	static inline std::string name = "program";
	static inline std::string parent = "gl";
	static inline std::string description = "Low-level shaders and programs";

	static void apply(sol::table mod) {
#define QUICKBIND_F(name) mod[#name] = gl ## name
#define	QUICKBIND_C(name) mod[#name] = GL_##name

		QUICKBIND_F(CreateProgram);
		QUICKBIND_F(CreateShader);
		QUICKBIND_C(VERTEX_SHADER);
		QUICKBIND_C(FRAGMENT_SHADER);
		QUICKBIND_C(COMPUTE_SHADER);

		mod["ShaderSource"] = [](GLuint program, std::string source) {
			const char* src = source.c_str();
			glShaderSource(program, 1, &src, NULL);
		};

		QUICKBIND_F(CompileShader);

		mod["MakeShader"] = [](GLenum type, std::string source) {
			GLuint shader = glCreateShader(type);
			const char* src = source.c_str();

			glShaderSource(shader, 1, &src, NULL);
			glCompileShader(shader);
		};

		QUICKBIND_F(AttachShader);
		QUICKBIND_F(LinkProgram);

		mod["Uniform"] = sol::overload(
			[](refrakt::arg_t arg) {
				std::cout << refrakt::type_string(arg) << std::endl;
			},
			[](sol::object) {
				spdlog::get("console")->error("gl.Uniform called with non arg_t type");
			}
		);
	}
};