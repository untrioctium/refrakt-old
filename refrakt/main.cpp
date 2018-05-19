#include "imgui.h"
#include "imgui-SFML.h"
#include "sol.hpp"
#include "json.hpp"
#include "RtMidi.h"
#include <GL/glew.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>
#include <string>
#include <fstream>

#include "GLtypes.hpp"

#include "RefraktProgram.hpp"

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
	} else printf("(C%d) %s: [%s: %d] (%f since last event)\n", channel, names[status].c_str(), byte1[status].c_str(), data1, time);
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

	std::ifstream t("programs\\test.frag");
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

int main( int argc, char** argv )
{
	/*midi stuff*/
	json settings;
	std::ifstream("settings.json") >> settings;

	sf::RenderWindow window(
		sf::VideoMode(
			settings.value<unsigned int>("width", 1280),
			settings.value<unsigned int>("height", 720)
		),
		"refrakt",
		settings.value("fullscreen", false) ? sf::Style::Fullscreen : sf::Style::Default,
		sf::ContextSettings( // OpenGL settings
			settings.value("/ogl/depth_buffer_bits"_json_pointer,   24), // depth buffer bits
			settings.value("/ogl/stencil_buffer_bits"_json_pointer,  8), // stencil buffer bits
			settings.value("/ogl/antialiasing"_json_pointer,         0), // antialiasing level
			4, // OpenGL major version
			3, // OpenGL minor version
			sf::ContextSettings::Attribute::Debug
		)
	);
	window.setFramerateLimit(settings.value<unsigned int>("framerate", 60));

	glewExperimental = true;
	glewInit();

	GLuint handle = genRenderProg();

	window.setActive();
    ImGui::SFML::Init(window);

    auto pgm = RefraktProgram::load("escape.lua");



    char cmd_buf[256] = {'\0'};

    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed || 
				(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Key::Escape )) {
                window.close();
				return 0;
            }
        }

		window.clear();

        ImGui::SFML::Update(window, deltaClock.restart());
        //ImGui::ShowTestWindow();

        ImGui::Begin("Console");
        if(ImGui::InputText("Lua Command", cmd_buf, 256, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            ImGui::SetKeyboardFocusHere(-1);
            std::string command = cmd_buf;
            cmd_buf[0] = '\0';

			std::cout << "$: " << command << std::endl;

            std::string result = pgm->getLuaState().script(command, [](lua_State* L, sol::protected_function_result pfr) {
              return pfr;
            });

			if( result != "nil\n" ) std::cout << result << std::endl;
        }
        ImGui::End();

		pgm->drawGui();

		//pgm->lua_state.script("lol:gui( format )");

		auto pushToOpenGL = [pgm, handle](auto arg, auto type, auto function) {
			function(glGetUniformLocation(handle, arg), 1, &(pgm->get<decltype(type)>(arg)));
		};

		glBindVertexArray(vertArray);
		glBindBuffer(GL_ARRAY_BUFFER, posBuf);
		glUseProgram(handle);
		sf::Vector2 size = window.getSize();
		glViewport(0, 0, size.x, size.y);
		pushToOpenGL("center", vec2(), glUniform2fv);
		pushToOpenGL("scale", rfkt::float_t(), glUniform1fv);
		pushToOpenGL("exponent", vec2(), glUniform2fv);
		pushToOpenGL("escape_radius", rfkt::float_t(), glUniform1fv);
		pushToOpenGL("max_iterations", rfkt::uint32_t(), glUniform1uiv);
		pushToOpenGL("julia", vec2(), glUniform2fv);
		pushToOpenGL("julia_c", vec2(), glUniform2fv);
		pushToOpenGL("burning_ship", vec2(), glUniform2fv);
		glUniform1f(glGetUniformLocation(handle, "surface_ratio"), float(size.y) / float(size.x));

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		window.pushGLStates();
		ImGui::SFML::Render(window);
		window.popGLStates();

		window.display();


    }

    ImGui::SFML::Shutdown();

    return 0;
}
