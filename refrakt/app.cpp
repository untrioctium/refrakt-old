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

#include "json.hpp"
#include "texture.hpp"

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

		window.create(
			sf::VideoMode{
				settings.value<unsigned int>("width", 1280),
				settings.value<unsigned int>("height", 720)
			},
			"refrakt",
			settings.value("fullscreen", false)? sf::Style::Fullscreen: sf::Style::Default,
			sf::ContextSettings{
				settings.value("/ogl/depth_buffer_bits"_json_pointer, 24U), // depth buffer bits
				settings.value("/ogl/stencil_buffer_bits"_json_pointer, 8U), // stencil buffer bits
				settings.value("/ogl/antialiasing"_json_pointer, 0U), // antialiasing level
				4, // OpenGL major version
				3, // OpenGL minor version
				sf::ContextSettings::Attribute::Debug
			}
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
		refrakt::texture_pool pool;

		pool.request({ 4096, 4096, refrakt::texture::format::Float, 4, 1 });
		pool.request({ 1280, 1024, refrakt::texture::format::Float, 4, 1 });
		pool.request({ 1280, 1024, refrakt::texture::format::SignedInt, 4, 1 });

		std::size_t usage = 0;
		refrakt::events::gl_calc_vram_usage::fire(usage);
		std::cout << "vram usage: " << usage / 1048576 << std::endl;

		while (frame()) {}
		window.close();
		return 0;
	}

	bool frame() {
		sf::Event event;
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed ||
				(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Key::Escape)) {
				return false;
			}
		}

		return true;
	}

private:
	sf::RenderWindow window;
};

int main(int argc, char** argv) {
	std::vector<std::string> arg;
	for (int i = 0; i < argc; i++) arg.push_back(argv[i]);

	app rfkt;
	rfkt.init(arg);
	return rfkt.run();
}
