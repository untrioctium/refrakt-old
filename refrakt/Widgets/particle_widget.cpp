#include <GL/glew.h>
#include <thread>
#include <iostream>

#include "type_helpers.hpp"
#include "widget.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class particle_widget : public refrakt::widget::Registrar<particle_widget>, public refrakt::events::gl_was_reset::observer {
private:
	GLuint prog_;
	GLuint fbo_;

	void gen_buffers() {

		// generate framebuffer
		glGenFramebuffers(1, &fbo_);
	}

public:
	particle_widget() {}

	void on_notify(refrakt::events::gl_was_reset::tag) {
		gen_buffers();
	}

	void setup(nlohmann::json config) {
		refrakt::widget::setup(config);

		static const char* vp_src = R"shader(
			#version 430
			out vec4 frag_color;
	
			uniform ivec2 dim;
			uniform mat4 view;

			uniform sampler2D pos;
			uniform sampler2D col;

			void main() {
				vec2 vertex_pos = vec2( (gl_VertexID % dim.x) / float(dim.x), (gl_VertexID / dim.x) / float(dim.y));
				vec3 v = texture(pos, vertex_pos).xyz;
				frag_color = texture(col, vertex_pos);

				if( isnan( dot(frag_color, vec4(1.0))) ) frag_color = vec4(0.0, 0.0, 0.0, 0.0);

				gl_Position = view * vec4(v.xyz, 1.0);
			}

		)shader";

		static const char* fp_src = R"shader(
			#version 430
			in vec4 frag_color;
			layout(location=0) out vec4 color;

			void main() { color = frag_color; }
		)shader";

		prog_ = glCreateProgram();

		GLuint vp = glCreateShader(GL_VERTEX_SHADER);
		GLuint fp = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(vp, 1, &vp_src, NULL);
		glShaderSource(fp, 1, &fp_src, NULL);
		glCompileShader(vp);

		int rvalue;
		glGetShaderiv(vp, GL_COMPILE_STATUS, &rvalue);
		if (!rvalue) {
			GLchar log[10240];
			GLsizei length;
			glGetShaderInfoLog(fp, 10239, &length, log);
			std::cout << log << std::endl;
		}

		glAttachShader(prog_, vp);
		glCompileShader(fp);
		glAttachShader(prog_, fp);

		glLinkProgram(prog_);

		glGetProgramiv(prog_, GL_LINK_STATUS, &rvalue);
		if (!rvalue) {
			GLchar log[10240];
			GLsizei length;
			glGetProgramInfoLog(prog_ , 10239, &length, log);
			std::cout << log << std::endl;

		}

		gen_buffers();
	}

	void run(refrakt::widget::param_t& input, refrakt::widget::param_t& output) const {
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		glUseProgram(prog_);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

		int bound_input_textures = 0;
		int total_elements = 0;

		for (auto& kv : input) {
			std::visit([name = kv.first, this, &bound_input_textures, &total_elements](auto&& v) {
				using type = std::decay_t<decltype(v)>;

				if constexpr(refrakt::is_static_array_v<type>)
					refrakt::type_helpers::opengl::push(prog_, name, v);
				else if constexpr(std::is_same_v<type, refrakt::texture_handle>) {
					if (name == "pos") {
						total_elements = v->info().w * v->info().h;
						glUniform2i(glGetUniformLocation(prog_, "dim"), v->info().w, v->info().h);
					}

					GLint location = glGetUniformLocation(prog_, name.c_str());
					glUniform1i(location, bound_input_textures);
					glActiveTexture(GL_TEXTURE0 + bound_input_textures);
					glBindTexture(GL_TEXTURE_2D, v->handle());
					bound_input_textures++;
				}

			}, kv.second);
		}

		auto handle = std::get<refrakt::texture_handle>(output["result"]);

		glm::mat4 projection = glm::perspective(glm::radians(70.0f), float(handle->info().w) / handle->info().h, 0.1f, 100.0f) * glm::lookAt(
			glm::vec3(0, 0, -3.0), // Camera is at (4,3,3), in World Space
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		) * glm::mat4(1.0f);

		glUniformMatrix4fv(glGetUniformLocation(prog_, "view"), 1, false, glm::value_ptr(projection));

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, handle->handle(), 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		if (std::get<refrakt::uint32_t>(input["clear"])[0] == 1) {
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
		}
		glViewport(0, 0, handle->info().w, handle->info().h);

		glDrawArrays(GL_POINTS, 0, total_elements);
		glUseProgram(0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(viewport[0], viewport[1], viewport[2], viewport[2]);

		glDisable(GL_BLEND);
	}
};