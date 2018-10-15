#include "widget.hpp"
#include <GL/glew.h>
#include "type_helpers.hpp"
#include <thread>

#include <iostream>

class particle_widget : public refrakt::widget::Registrar<particle_widget>, public refrakt::events::gl_was_reset::observer {
private:
	std::pair<std::size_t, std::size_t> size_;

	GLuint prog_;
	GLuint vertex_buffer_;
	GLuint vertex_array_;
	GLuint fbo_;

	std::vector<float> vertex_data;

	void gen_buffers() {
	

		glGenVertexArrays(1, &vertex_array_);
		glBindVertexArray(vertex_array_);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_data.size(), vertex_data.data(), GL_STATIC_DRAW);
		GLuint pos_ptr = glGetAttribLocation(prog_, "vertex_pos");
		glVertexAttribPointer(pos_ptr, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(pos_ptr);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

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

		size_ = { config["widget"]["size"][0].get<std::size_t>(),config["widget"]["size"][1].get<std::size_t>() };

		std::size_t arr_size = size_.first * size_.second * 2;
		vertex_data.resize(arr_size);

		float dx = 1.0 / (size_.first - 1);
		float dy = 1.0 / (size_.second - 1);

		unsigned int total_threads = std::thread::hardware_concurrency();
		unsigned int per_thread = arr_size / 2 / std::thread::hardware_concurrency();
		unsigned int stragglers = arr_size / 2 % std::thread::hardware_concurrency();
		std::vector<std::thread> threads;

		for (int i = 0; i < total_threads; i++) {
			threads.push_back(std::thread{ [this, start = i * per_thread, end = (i == total_threads - 1) ? arr_size / 2 : (i + 1) * per_thread]() {
				for (int i = start; i < end; i++) {
					vertex_data[i * 2] = i % size_.first / float(size_.first);
					vertex_data[i * 2 + 1] = i / size_.first / float(size_.second);
				}
			}});
		}

		for (auto& t : threads) t.join();

		static const char* vp_src = R"shader(
			#version 430
			in vec2 vertex_pos;
			out vec4 frag_color;

			//uniform mat4 view;
			uniform sampler2D pos;
			//uniform sampler2d col;

			void main() {
				frag_color = vec4(vertex_pos.x, vertex_pos.y, 0.25, 1.0);//texture(col, vertex_pos);
				gl_Position = vec4(texture(pos, vertex_pos).xyz, 1.0);
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

		glGenBuffers(1, &vertex_buffer_);
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

		auto handle = std::get<refrakt::texture_handle>(output["result"]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, handle->handle(), 0);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, handle->info().w, handle->info().h);

		glBindVertexArray(vertex_array_);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
		glDrawArrays(GL_POINTS, 0, size_.first * size_.second);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(viewport[0], viewport[1], viewport[2], viewport[2]);

		glDisable(GL_BLEND);
	}
};