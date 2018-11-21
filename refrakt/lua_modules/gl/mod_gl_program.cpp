#include <GL/glew.h>
#include <spdlog/spdlog.h>
#include <glm/gtc/type_ptr.hpp>

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

		mod["create_program"] = glCreateProgram;
		mod["create_shader"] = glCreateShader;
		mod["VERTEX_SHADER"] = GL_VERTEX_SHADER;
		mod["FRAGMENT_SHADER"] = GL_FRAGMENT_SHADER;
		mod["COMPUTE_SHADER"] = GL_COMPUTE_SHADER;

		mod["shader_source"] = [](GLuint shader, std::string source) {
			const char* src = source.c_str();
			glShaderSource(shader, 1, &src, NULL);
		};

		mod["compile_shader"] = [](GLuint shader) {
			glCompileShader(shader);
			GLint result;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
			return result == GL_TRUE;
		};

		mod["make_shader"] = [](GLenum type, std::string source) {
			GLuint shader = glCreateShader(type);
			const char* src = source.c_str();

			glShaderSource(shader, 1, &src, NULL);
			GLint result;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
			return std::tuple{ shader, result == GL_TRUE };
		};

		mod["get_shader_info_log"] = [](GLuint shader) {
			GLint max_length = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

			std::string result;
			result.resize(max_length);
			glGetShaderInfoLog(shader, max_length, 0, result.data());
			return result;
		};

		mod["attach_shader"] = glAttachShader;
		mod["link_program"] = [](GLuint program) {
			glLinkProgram(program);
			GLint is_linked;
			glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
			return is_linked == GL_TRUE;
		};

		mod["make_program"] = [](sol::variadic_args shaders) {
			GLuint program = glCreateProgram();
			for (auto shader : shaders) glAttachShader(program, shader);
			glLinkProgram(program);
			GLint is_linked;
			glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
			return std::tuple{ program, is_linked == GL_TRUE };
		};

		mod["get_program_info_log"] = [](GLuint program) {
			GLint max_length = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);

			std::string result;
			result.resize(max_length);
			glGetProgramInfoLog(program, max_length, 0, result.data());
			return result;
		};

		mod["get_uniform_location"] = [](GLuint program, std::string name) {
			return glGetUniformLocation(program, name.c_str());
		};

		mod["get_frag_data_location"] = [](GLuint program, std::string name) {
			return glGetFragDataLocation(program, name.c_str());
		};

		mod["use_program"] = glUseProgram;
		mod["clear_program"] = []() { glUseProgram(0); };

		mod["uniform"] = sol::overload(
			[](GLuint program, const std::string& name, refrakt::texture arg, sol::this_state L) {

				auto log = spdlog::get("console");
				log->error("{}.uniform called with texture type.", parent);
				log->error("Bind it to a texture unit and pass the unit as a uint32 instead.");

				luaL_traceback(L, L, NULL, 1);
				log->error("{}", lua_tostring(L, -1));
				lua_pop(L, 1);
			},
			[](GLuint program, const std::string& name, refrakt::arg_t arg) {
				refrakt::type_helpers::opengl::push(program, name, arg);
			},
			[](GLuint program, const std::string& name, sol::object arg) {
				spdlog::get("console")->error("{}.uniform called with non argument type", parent);
			}
		);

		mod["COMPUTE_WORK_GROUP_SIZE"] = GL_COMPUTE_WORK_GROUP_SIZE;
		mod["get_program"] = [](GLuint program, GLenum pname) 
			-> std::variant<GLint, refrakt::ivec3> {
			if (pname == GL_COMPUTE_WORK_GROUP_SIZE) {
				refrakt::ivec3 result;
				glGetProgramiv(program, pname, glm::value_ptr(result));
				return result;
			}
			else {
				GLint result;
				glGetProgramiv(program, pname, &result);
				return result;
			}
		};

		mod["dispatch_compute"] = sol::overload(
			glDispatchCompute,
			[](refrakt::uvec3 size) {
				glDispatchCompute(size.x, size.y, size.z);
			}
		);

		mod["UNIFORM"] = GL_UNIFORM;
		mod["UNIFORM_BLOCK"] = GL_UNIFORM_BLOCK;
		mod["PROGRAM_INPUT"] = GL_PROGRAM_INPUT;
		mod["PROGRAM_OUTPUT"] = GL_PROGRAM_OUTPUT;
		mod["BUFFER_VARIABLE"] = GL_BUFFER_VARIABLE;
		mod["VERTEX_SUBROUTINE"] = GL_VERTEX_SUBROUTINE;
		mod["COMPUTE_SUBROUTINE"] = GL_COMPUTE_SUBROUTINE;
		mod["GEOMETRY_SUBROUTINE"] = GL_GEOMETRY_SUBROUTINE;
		mod["FRAGMENT_SUBROUTINE"] = GL_FRAGMENT_SUBROUTINE;
		mod["SHADER_STORAGE_BLOCK"] = GL_SHADER_STORAGE_BLOCK;
		mod["TESS_CONTROL_SUBROUTINE"] = GL_TESS_CONTROL_SUBROUTINE;
		mod["VERTEX_SUBROUTINE_UNIFORM"] = GL_VERTEX_SUBROUTINE_UNIFORM;
		mod["TRANSFORM_FEEDBACK_BUFFER"] = GL_TRANSFORM_FEEDBACK_BUFFER;
		mod["TESS_EVALUATION_SUBROUTINE"] = GL_TESS_EVALUATION_SUBROUTINE;
		mod["COMPUTE_SUBROUTINE_UNIFORM"] = GL_COMPUTE_SUBROUTINE_UNIFORM;
		mod["TRANSFORM_FEEDBACK_VARYING"] = GL_TRANSFORM_FEEDBACK_VARYING;
		mod["TRANSFORM_FEEDBACK_VARYING"] = GL_TRANSFORM_FEEDBACK_VARYING;
		mod["GEOMETRY_SUBROUTINE_UNIFORM"] = GL_GEOMETRY_SUBROUTINE_UNIFORM;
		mod["FRAGMENT_SUBROUTINE_UNIFORM"] = GL_FRAGMENT_SUBROUTINE_UNIFORM;
		mod["TESS_CONTROL_SUBROUTINE_UNIFORM"] = GL_TESS_CONTROL_SUBROUTINE_UNIFORM;
		mod["TESS_EVALUATION_SUBROUTINE_UNIFORM"] = GL_TESS_EVALUATION_SUBROUTINE_UNIFORM;

		mod["get_program_resource_index"] = glGetProgramResourceIndex;
	}
};