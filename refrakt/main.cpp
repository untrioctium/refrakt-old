#include "imgui.h"
#include "imgui-SFML.h"
#include <GL/glew.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>
#include <string>
#include <fstream>

#include "GLtypes.hpp"
#include "type_helpers.hpp"

#include "widget.hpp"

using json = nlohmann::json;

void checkErrors(std::string desc) {
	GLenum e = glGetError();
	if (e != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL error in \"%s\": %s (%d)\n", desc.c_str(), gluErrorString(e), e);
		std::cin.get();
		exit(20);
	}
}

void show_main_menu() {
	if (ImGui::BeginMainMenuBar()) {
		static sf::Clock fpsCounter;
		static float fpsAvg = 0.0;
		int lastTime = fpsCounter.restart().asMilliseconds();
		fpsAvg = .9f * fpsAvg + 100.0f / float((lastTime == 0) ? 1 : lastTime);

		ImGui::Text("Press ~ to hide guis. Press F1 to toggle fullscreen. %.0f FPS", fpsAvg);

		ImGui::EndMainMenuBar();
	}
}

class EscapeWidget : public refrakt::widget {
public:
	void initialize() {
		render_prog_ = glCreateProgram();
		GLuint vp = glCreateShader(GL_VERTEX_SHADER);
		GLuint fp = glCreateShader(GL_FRAGMENT_SHADER);

		const char *vpSrc[] = {
			"#version 430\n",
			"in vec2 pos;\
		 out vec2 texCoord;\
		 void main() {\
			 texCoord = pos;\
			 gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);\
		 }"
		};

		std::ifstream t("programs\\escape.frag");
		std::string fpSrc((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
		const char* src = fpSrc.c_str();

		glShaderSource(vp, 2, vpSrc, NULL);
		glShaderSource(fp, 1, &src, NULL);

		glCompileShader(vp);
		int rvalue;
		glGetShaderiv(vp, GL_COMPILE_STATUS, &rvalue);
		if (!rvalue) {
			fprintf(stderr, "Error in compiling vp\n");
			exit(30);
		}
		glAttachShader(render_prog_, vp);

		glCompileShader(fp);
		glGetShaderiv(fp, GL_COMPILE_STATUS, &rvalue);
		if (!rvalue) {
			fprintf(stderr, "Error in compiling the compute shader\n");
			GLchar log[10240];
			GLsizei length;
			glGetShaderInfoLog(fp, 10239, &length, log);
			fprintf(stderr, "Compiler log:\n%s\n", log);
			std::cin.get();
			exit(40);
		}

		glAttachShader(render_prog_, fp);

		glBindFragDataLocation(render_prog_, 0, "color");
		glLinkProgram(render_prog_);

		glGetProgramiv(render_prog_, GL_LINK_STATUS, &rvalue);
		if (!rvalue) {
			fprintf(stderr, "Error in linking compute shader program\n");
			GLchar log[10240];
			GLsizei length;
			glGetProgramInfoLog(render_prog_, 10239, &length, log);
			fprintf(stderr, "Linker log:\n%s\n", log);
			std::cin.get();
			exit(41);
		}

		glGenVertexArrays(1, &vert_array_);
		glBindVertexArray(vert_array_);

		glGenBuffers(1, &pos_buf_);
		glBindBuffer(GL_ARRAY_BUFFER, pos_buf_);
		float data[] = {
			-1.0f, -1.0f,
			-1.0f, 1.0f,
			1.0f, -1.0f,
			1.0f, 1.0f
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, data, GL_STREAM_DRAW);
		GLint posPtr = glGetAttribLocation(render_prog_, "pos");
		glVertexAttribPointer(posPtr, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(posPtr);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		checkErrors("Render shaders");
	}

	auto create_parameter_set() -> refrakt::widget::parameter_set {
		return refrakt::widget::parameter_set
		{
			{"center", refrakt::vec2{}},
			{"scale", refrakt::float_t{1.0} },
			{"hue_shift", refrakt::float_t{0.0} },
			{"hue_stretch", refrakt::float_t{1.0} },
			{"exponent", refrakt::vec2{2.0, 0.0} },
			{ "escape_radius", refrakt::float_t{4.0} },
			{ "max_iterations", refrakt::uint32_t{100} },
			{ "julia", refrakt::vec2{0.0, 0.0} },
			{ "julia_c", refrakt::vec2{0.0, 0.0} },
			{ "burning_ship", refrakt::vec2{0.0, 0.0} },
			{ "hq_mode", refrakt::uint32_t{0} },
			{ "surface_ratio", refrakt::float_t{} },
			{ "offset", refrakt::vec2{} }
		};
	}

	auto operator()(const refrakt::widget::parameter_set& p) -> refrakt::widget::parameter_set {

		glUseProgram(render_prog_);
		checkErrors("use program");
		glBindVertexArray(vert_array_);
		checkErrors("bind array");
		glBindBuffer(GL_ARRAY_BUFFER, pos_buf_);
		checkErrors("bind buffer");
		for (auto& kv : p) {
			refrakt::opengl::type_helpers::push(glGetUniformLocation(render_prog_, kv.first.c_str()), kv.second);
		}

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		checkErrors("draw array");
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);


		return {};
	}

	bool validate(const refrakt::widget::parameter_set &) { return true;  }

private:
	GLuint render_prog_;
	GLuint vert_array_;
	GLuint pos_buf_;
};

int main(int argc, char** argv)
{
	json settings;
	std::ifstream("settings.json") >> settings;

	sf::VideoMode screenInfo(
		settings.value<unsigned int>("width", 1280),
		settings.value<unsigned int>("height", 720)
	);

	bool fullscreen = settings.value("fullscreen", false);

	auto windowStyle = fullscreen ? sf::Style::Fullscreen : sf::Style::Default;
	sf::ContextSettings ctxSettings(
		settings.value("/ogl/depth_buffer_bits"_json_pointer, 24), // depth buffer bits
		settings.value("/ogl/stencil_buffer_bits"_json_pointer, 8), // stencil buffer bits
		settings.value("/ogl/antialiasing"_json_pointer, 0), // antialiasing level
		4, // OpenGL major version
		3, // OpenGL minor version
		sf::ContextSettings::Attribute::Debug
	);

	sf::RenderWindow window(
		screenInfo,
		"refrakt",
		windowStyle,
		ctxSettings
	);
	window.setFramerateLimit(settings.value<unsigned int>("framerate", 60));

	glewExperimental = true;
	glewInit();

	window.setActive();
	ImGui::SFML::Init(window);

	EscapeWidget w;
	w.initialize();

	auto param = w.create_parameter_set();

	char cmd_buf[256] = { '\0' };

	bool showGui = true;

	sf::Clock deltaClock;

	std::cout << refrakt::type_string(refrakt::arg_t(refrakt::vec2())) << std::endl;
	
	float fpsAvg = 0;

	while (window.isOpen()) {

		sf::Event event;

		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed ||
				(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Key::Escape)) {
				window.close();
				return 0;
			}

			if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Key::Tilde)
				showGui ^= true;

			if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Key::F1) {
				if( fullscreen )
					window.create(screenInfo, "refrakt", sf::Style::Default, ctxSettings);
				else
				{
					sf::Vector2 size = window.getSize();
					screenInfo.width = size.x;
					screenInfo.height = size.y;
					window.create(sf::VideoMode::getDesktopMode(), "refrakt", sf::Style::None, ctxSettings);
				}
				fullscreen ^= true;
			}
		}


		window.clear();


		ImGui::SFML::Update(window, deltaClock.restart());

		//ImGui::ShowTestWindow();
		//show_lua_console(pgm->getLuaState());
		if (showGui)
		{
			show_main_menu();
		}
		//pgm->lua_state.script("lol:gui( format )");
		
		auto size = window.getSize();

		param["surface_ratio"] = refrakt::float_t{ float(size.x) / float(size.y) };
		param["offset"] = refrakt::vec2{ 1.0f/float(size.x), 1.0f/float(size.y) };
		glViewport(0, 0, size.x, size.y);
		w(param);

		if (showGui) {
			window.pushGLStates();
			ImGui::SFML::Render(window);
			window.popGLStates();


		} else ImGui::EndFrame();
		window.display();
	}

	ImGui::SFML::Shutdown();

	return 0;
}
