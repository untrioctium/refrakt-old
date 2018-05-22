#include "imgui.h"
#include "imgui-SFML.h"
#include "sol.hpp"
#include "RtMidi.h"
#include <GL/glew.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>
#include <string>
#include <fstream>

#include "GLtypes.hpp"

#include "RefraktWidget.hpp"

using json = nlohmann::json;

void interpret_midi(std::vector<uint8_t>& message, double time)
{
	uint8_t status = (message[0] >> 4) - 8;
	uint8_t data1 = message[1];
	uint8_t channel = message[0] & 15;

	static std::string names[] = { "Note off", "Note on", "Key pressure", "Controller change", "Program change", "Channel pressure", "Pitch bend" };
	static std::string byte1[] = { "key", "key", "key", "controller", "preset", "pressure", "fine" };
	static std::string byte2[] = { "velocity", "velocity", "pressure", "value", "null", "null", "coarse" };

	if (message.size() == 3)
	{
		uint8_t data2 = message[2];
		printf("(C%d) %s: [%s: %d, %s: %d] (%f since last event)\n", channel, names[status].c_str(), byte1[status].c_str(), data1, byte2[status].c_str(), data2, time);
	}
	else printf("(C%d) %s: [%s: %d] (%f since last event)\n", channel, names[status].c_str(), byte1[status].c_str(), data1, time);
}

void checkErrors(std::string desc) {
	GLenum e = glGetError();
	if (e != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL error in \"%s\": %s (%d)\n", desc.c_str(), gluErrorString(e), e);
		exit(20);
	}
}

GLuint vertArray;
GLuint posBuf;

GLuint genRenderProg() {
	GLuint progHandle = glCreateProgram();
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
	glAttachShader(progHandle, vp);

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

	glAttachShader(progHandle, fp);

	glBindFragDataLocation(progHandle, 0, "color");
	glLinkProgram(progHandle);

	glGetProgramiv(progHandle, GL_LINK_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in linking compute shader program\n");
		GLchar log[10240];
		GLsizei length;
		glGetProgramInfoLog(progHandle, 10239, &length, log);
		fprintf(stderr, "Linker log:\n%s\n", log);
		std::cin.get();
		exit(41);
	}

	glGenVertexArrays(1, &vertArray);
	glBindVertexArray(vertArray);

	glGenBuffers(1, &posBuf);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf);
	float data[] = {
		-1.0f, -1.0f,
		-1.0f, 1.0f,
		1.0f, -1.0f,
		1.0f, 1.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, data, GL_STREAM_DRAW);
	GLint posPtr = glGetAttribLocation(progHandle, "pos");
	glVertexAttribPointer(posPtr, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posPtr);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	checkErrors("Render shaders");
	return progHandle;
}

void show_lua_console(sol::state& state) {
	ImGui::Begin("Lua Console");

	static char command_buffer[1024] = { 0 };

	if (ImGui::InputText("##command", command_buffer, 256, ImGuiInputTextFlags_EnterReturnsTrue))
	{
		ImGui::SetKeyboardFocusHere(-1);
		std::string command = command_buffer;
		command_buffer[0] = '\0';

		std::cout << "$: " << command << std::endl;

		std::string result = state.script(command, [](lua_State* L, sol::protected_function_result pfr) {
			return pfr;
		});

		if (result != "nil\n") std::cout << result << std::endl;
	}
	ImGui::End();
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

void show_program_parameters(RefraktWidget& pgm) {
	ImGui::Begin("Parameters", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove);

	if (ImGui::BeginMenuBar()) {
		if (ImGui::Button("Copy to Clipboard"))
			ImGui::SetClipboardText(pgm.serialize().c_str());

		ImGui::SameLine();

		if (ImGui::Button("Load from Clipboard"))
			pgm.deserialize(ImGui::GetClipboardText());

		ImGui::EndMenuBar();
	}
	pgm.drawGui();

	ImGui::SetWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - ImGui::GetWindowHeight()));

	ImGui::End();
}

class MidiControllers {
public:

	std::uint8_t get(std::uint8_t channel, std::uint8_t controller) {
		std::uint16_t id = mapToID(channel, controller);

		if (states_.count(id) == 0) return 0;
		return states_[id];
	}

	void set(std::uint8_t channel, std::uint8_t controller, std::uint8_t value) {
		states_[mapToID(channel, controller)] = value;
	}

private:

	static std::uint16_t mapToID(std::uint8_t channel, std::uint8_t controller) {
		return (std::uint16_t(channel) << 8) + controller;
	}
	std::map<std::uint16_t, std::uint8_t> states_;
};

int main(int argc, char** argv)
{
	/*midi stuff*/
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

	GLuint handle = genRenderProg();

	window.setActive();
	ImGui::SFML::Init(window);

	auto pgm = RefraktWidget::load("escape.lua");
	char cmd_buf[256] = { '\0' };

	bool showGui = true;

	sf::Clock deltaClock;
	auto lol = rfkt::arg_t(bvec4(false));
	auto jej = lol.ptr<bool>();
	jej[0] = 2;

	std::cout << lol.serialize() << std::endl;
	std::cout << rfkt::arg_t::create("vec2").serialize() << std::endl;
	
	float fpsAvg = 0;

	RtMidiIn *midiin = new RtMidiIn();
	midiin->openPort(0);
	midiin->ignoreTypes(false, false, false);

	MidiControllers m;

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
				handle = genRenderProg();
			}
		}

		std::vector<unsigned char> message;
		int nBytes, i;
		double stamp;

		while (true) {
			stamp = midiin->getMessage(&message);
			if (message.size() == 0) break;
			if (message.size() != 3) continue;
			interpret_midi(message, stamp);
			int status = (message[0] >> 4) - 8;
			if (status != 3) continue;

			uint8_t channel = message[0] & 0b1111;

			m.set(channel, message[1], message[2]);
		}

		window.clear();


		ImGui::SFML::Update(window, deltaClock.restart());

		//ImGui::ShowTestWindow();
		//show_lua_console(pgm->getLuaState());
		if (showGui)
		{
			show_main_menu();
			show_program_parameters(*pgm);
		}
		//pgm->lua_state.script("lol:gui( format )");

		auto pushToOpenGL = [pgm, handle](auto arg, auto type, auto function, auto modifier) {
			using T = decltype(type);
			T twiddled = modifier(pgm->get<T>(arg));
			function(glGetUniformLocation(handle, arg), 1, &twiddled);
		};

		glBindVertexArray(vertArray);
		glBindBuffer(GL_ARRAY_BUFFER, posBuf);
		glUseProgram(handle);

		sf::Vector2 size = window.getSize();

		auto passthrough = [](auto a) { return a; };
		auto twiddle = [&m](vec2 a){
			a[0] = 2.0f*(float(m.get(1, 95)) / 127.0f - .5f) * .01f + a[0];
			a[1] = 2.0f*(float(m.get(1, 62)) / 127.0f - .5f) * .01f + a[1];
			return a;
		};

		glViewport(0, 0, size.x, size.y);
		pushToOpenGL("center", vec2(), glUniform2fv, passthrough);
		pushToOpenGL("scale", rfkt::float_t(), glUniform1fv, passthrough);
		pushToOpenGL("exponent", vec2(), glUniform2fv, twiddle);
		pushToOpenGL("escape_radius", rfkt::float_t(), glUniform1fv, passthrough);
		pushToOpenGL("hue_shift", rfkt::float_t(), glUniform1fv, passthrough);
		pushToOpenGL("hue_stretch", rfkt::float_t(), glUniform1fv, passthrough);
		pushToOpenGL("max_iterations", rfkt::uint32_t(), glUniform1uiv, passthrough);
		pushToOpenGL("julia", vec2(), glUniform2fv, passthrough);
		pushToOpenGL("julia_c", vec2(), glUniform2fv,twiddle);
		pushToOpenGL("burning_ship", vec2(), glUniform2fv, passthrough);
		glUniform1f(glGetUniformLocation(handle, "surface_ratio"), float(size.y) / float(size.x));

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

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
