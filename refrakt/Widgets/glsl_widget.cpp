#include <GL/glew.h>

#include <fstream>
#include <streambuf>
#include <string>
#include <iostream>

#include "type_helpers.hpp"
#include "widget.hpp"

class glsl_widget : 
	public refrakt::widget::Registrar<glsl_widget>, 
	public refrakt::events::gl_was_reset::observer {
private:
	GLuint prog_;
	GLuint fbo_;

	std::string shader_src_;

public:

	glsl_widget() {}

	void setup(nlohmann::json config) {
		refrakt::widget::setup(config);

		std::ifstream s{ "widgets/" + config["widget"]["source"].get<std::string>() };
		shader_src_ = std::string{ std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>() };

		// generate shader program
		prog_ = glCreateProgram();

		GLuint vp = glCreateShader(GL_VERTEX_SHADER);
		GLuint fp = glCreateShader(GL_FRAGMENT_SHADER);

		static const char* vp_src = R"shader(
			#version 430
			out vec2 pos;

			const vec2 verts[4] = vec2[](
				vec2(-1.0f, -1.0f),
				vec2(-1.0f, 1.0f),
				vec2(1.0f, -1.0f),
				vec2(1.0f, 1.0f)
			);

			void main() {
				pos = verts[gl_VertexID] * 0.5 + 0.5;
				gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
			}
		)shader";

		const char* fp_src = shader_src_.c_str();

		glShaderSource(vp, 1, &vp_src, NULL);
		glShaderSource(fp, 1, &fp_src, NULL);
		glCompileShader(vp);

		glAttachShader(prog_, vp);
		glCompileShader(fp);
		glAttachShader(prog_, fp);

		glLinkProgram(prog_);

		glGenFramebuffers(1, &fbo_);
	}

	void on_notify(refrakt::events::gl_was_reset::tag) {
		glGenFramebuffers(1, &fbo_);
		std::cout << "Widget " << this << " recreating framebuffer" << std::endl;
	}

	void run(refrakt::widget::param_t& input, refrakt::widget::param_t& output) const {

		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		glUseProgram(prog_);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

		int bound_input_textures = 0;
		for (auto& kv : input) {
			std::visit([name = kv.first, this, &bound_input_textures](auto&& v) {
				using type = std::decay_t<decltype(v)>;

				if constexpr(refrakt::is_static_array_v<type>)
					refrakt::type_helpers::opengl::push(prog_, name, v);
				else if constexpr(std::is_same_v<type, refrakt::texture_handle>) {
					GLint location = glGetUniformLocation(prog_, name.c_str());
					glUniform1i(location, bound_input_textures);
					glActiveTexture(GL_TEXTURE0 + bound_input_textures);
					glBindTexture(GL_TEXTURE_2D, v->handle());
					bound_input_textures++;
				}

			}, kv.second);
		}

		std::pair<std::size_t, std::size_t> smallest{ std::numeric_limits<std::size_t>::max(), std::numeric_limits<std::size_t>::max() };

		for (auto& kv : output) {
			std::visit([name = kv.first, this, &smallest](auto&& v){
				using type = std::decay_t<decltype(v)>;

				if constexpr(std::is_same_v<type, refrakt::texture_handle>) {
					std::pair<std::size_t, std::size_t> size = { v->info().w, v->info().h };
					if (size < smallest) smallest = size;

					GLuint location = glGetFragDataLocation(prog_, name.c_str());
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + location, GL_TEXTURE_2D, v->handle(), 0);
				}
			}, kv.second);
		}

		glViewport(0, 0, smallest.first, smallest.second);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glUseProgram(0);

		for (int i = 0; i < bound_input_textures; i++) {
			glActiveTexture(GL_TEXTURE0 + bound_input_textures);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(viewport[0], viewport[1], viewport[2], viewport[2]);
	}
};