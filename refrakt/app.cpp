#include "imgui.h"
#include "imgui-SFML.h"
#include <GL/glew.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include <istream>
#include <fstream>
#include <iostream>

#include "widget.hpp"
#include "type_helpers.hpp"
#include <Windows.h>

void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	const void *userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
};

class app {
public:
	void init(std::vector<std::string> argc) {
		nlohmann::json settings;
		std::ifstream("settings.json") >> settings;

		screen_info = sf::VideoMode{
			settings.value<unsigned int>("width", 1280),
			settings.value<unsigned int>("height", 720)
		};

		ctx = sf::ContextSettings(
				settings.value("/ogl/depth_buffer_bits"_json_pointer, 24), // depth buffer bits
				settings.value("/ogl/stencil_buffer_bits"_json_pointer, 8), // stencil buffer bits
				settings.value("/ogl/antialiasing"_json_pointer, 0), // antialiasing level
				4, // OpenGL major version
				3, // OpenGL minor version
				sf::ContextSettings::Attribute::Debug
		);

		bool fullscreen = settings.value("fullscreen", false);

		window.create(
			screen_info,
			"refrakt",
			fullscreen? sf::Style::Fullscreen: sf::Style::Default,
			ctx
		);

		window.setFramerateLimit(settings.value<unsigned int>("framerate", 60));

		glewExperimental = true;
		glewInit();

		window.setActive();
		ImGui::SFML::Init(window);

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}

	int run() {
		part = refrakt::widget::make("particle_widget");
		part->setup({
			{"widget", {{"size", { 512, 512 }}}}
		});

		fern = refrakt::widget::make("glsl_widget");
		fern->setup({
			{"widget", {{"source", "fern.frag"}}}
		});

		tone = refrakt::widget::make("glsl_widget");
		tone->setup({
			{ "widget",{ { "source", "tone.frag" } } }
			});

		blur = refrakt::widget::make("glsl_widget");
		blur->setup({
			{ "widget",{ { "source", "density.frag" } } }
		});

		alpha = refrakt::float_t{ 1.0 };
		max = refrakt::uint32_t{ 8 };
		sigma = refrakt::float_t{ 1.0 };
		exposure = refrakt::float_t{ 1.0 };
		preg = refrakt::float_t{ 1.0 };
		postg = refrakt::float_t{ 1.0 };

		setup_quad_drawer();
		while (frame()) {}
		window.close();
		return 0;
	}

	void setup_quad_drawer() {
		prog_ = glCreateProgram();

		GLuint vp = glCreateShader(GL_VERTEX_SHADER);
		GLuint fp = glCreateShader(GL_FRAGMENT_SHADER);

		static const char* vp_src = R"shader(
			#version 430
			in vec2 vertex_pos;
			out vec2 pos;

			void main() {
				pos = vertex_pos * 0.5 + 0.5;
				gl_Position = vec4(vertex_pos.x, vertex_pos.y, 0.0, 1.0);
			}
		)shader";

		static const char* fp_src = R"shader(
			#version 430
			in vec2 pos;
			layout(location=0) out vec4 color;

			uniform sampler2D tex;

			void main() {
				color = texture(tex, pos);
			}
		)shader";

		glShaderSource(vp, 1, &vp_src, NULL);
		glShaderSource(fp, 1, &fp_src, NULL);
		glCompileShader(vp);

		glAttachShader(prog_, vp);
		glCompileShader(fp);
		glAttachShader(prog_, fp);

		glLinkProgram(prog_);

		// generate vertex array quad
		glGenVertexArrays(1, &vertex_array_);
		glBindVertexArray(vertex_array_);
		glGenBuffers(1, &vertex_buffer_);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);

		static float vertex_data[] = {
			-1.0f, -1.0f,
			-1.0f, 1.0f,
			1.0f, -1.0f,
			1.0f, 1.0f
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, vertex_data, GL_STATIC_DRAW);
		GLuint pos_ptr = glGetAttribLocation(prog_, "vertex_pos");
		glVertexAttribPointer(pos_ptr, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(pos_ptr);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
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

	bool frame() {
		sf::Event event;
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed ||
				(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Key::Escape)) {
				return false;
			}

			if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Key::F1) {
				if (fullscreen)
					window.create(screen_info, "refrakt", sf::Style::Default, ctx);
				else
				{
					sf::Vector2 size = window.getSize();
					screen_info.width = size.x;
					screen_info.height = size.y;
					auto newsize = sf::VideoMode::getDesktopMode();
					newsize.height += 1;
					window.create(newsize, "refrakt", sf::Style::None, ctx);
				}
				fullscreen ^= true;
				refrakt::events::gl_was_reset::fire();
				setup_quad_drawer();
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		auto size = window.getSize();

		refrakt::type_helpers::imgui::display(max, "max width", { 1, 100 }, 1);
		refrakt::type_helpers::imgui::display(alpha, "alpha", { .01, 10 }, .001);
		refrakt::type_helpers::imgui::display(sigma, "sigma", { .01, 10 }, .001);
		refrakt::type_helpers::imgui::display(exposure, "exposure", { .01, 3 }, .01);
		refrakt::type_helpers::imgui::display(preg, "pre gamma", { .01, 3 }, .01);
		refrakt::type_helpers::imgui::display(postg, "post gamma", { .01, 3 }, .01);

		auto handle = pool.request(512, 512, refrakt::texture::format::Float, 3, 4);
		auto drawn = pool.request(1920, 1080, refrakt::texture::format::Float, 4, 4);
		auto blurred = pool.request(1920, 1080, refrakt::texture::format::Float, 4, 4);
		auto toned = pool.request(1920, 1080, refrakt::texture::format::Float, 4, 4);

		refrakt::widget::param_t in = {};
		refrakt::widget::param_t out = { {"result", handle} };
		fern->run(in, out);

		in = { {"pos", handle} };
		out = { { "result", drawn} };
		part->run(in, out);

		in = refrakt::widget::param_t{ {"tex", refrakt::arg_t{drawn}}, {"max_width", max}, {"alpha", alpha}, {"sig", sigma} };
		out = { {"result", blurred} };
		blur->run(in, out);

		in = { {"col", blurred}, {"exposure", exposure}, {"pre_gamma", preg}, {"post_gamma", postg} };
		out = { {"result", toned} };
		tone->run(in, out);


		show_main_menu();
		glViewport(0, 0, size.x, size.y);
		window.clear();

		glUseProgram(prog_);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, toned->handle());
		glUniform1i(glGetUniformLocation(prog_, "tex") , 0);

		glBindVertexArray(vertex_array_);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		window.pushGLStates();
		window.resetGLStates();
		ImGui::SFML::Render(window);
		window.popGLStates();

		window.display();

		return true;
	}

private:
	sf::RenderWindow window;
	sf::Clock deltaClock;
	sf::ContextSettings ctx;
	sf::VideoMode screen_info;
	bool fullscreen;

	GLuint prog_, vertex_buffer_, vertex_array_;

	refrakt::texture_pool pool;
	std::unique_ptr<refrakt::widget> part, fern, tone, blur;
	refrakt::arg_t alpha, max, sigma, exposure, preg, postg;

};

int main(int argc, char** argv) {
	std::vector<std::string> arg;
	for (int i = 0; i < argc; i++) arg.push_back(argv[i]);

	app rfkt;
	rfkt.init(arg);
	return rfkt.run();
}
