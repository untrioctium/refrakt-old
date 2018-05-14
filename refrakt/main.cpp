#include "imgui.h"
#include "imgui-SFML.h"
#include "sol.hpp"
#include "json.hpp"
#include <GL/glew.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>
#include <string>
#include <fstream>

#include "RefraktProgram.hpp"

using json = nlohmann::json;

int main( int argc, char** argv )
{

	json settings;
	std::ifstream("settings.json") >> settings;

	sf::RenderWindow window(
		sf::VideoMode(
			settings.value<unsigned int>("width", 1280),
			settings.value<unsigned int>("height", 720)
		),
		"refrakt",
		settings.value("fullscreen", false) ? sf::Style::Fullscreen : (sf::Style::Titlebar | sf::Style::Close),
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

	window.setActive();
    ImGui::SFML::Init(window);

    auto pgm = RefraktProgram::load("escape.lua");

	pgm->lua_state.script("lol = vec2(3.0)");
	pgm->lua_state.script("format = {name = 'lol', bounds={min=0.0, max=5.0}}");

    char cmd_buf[256] = {'\0'};

	int frame = 0;

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

        ImGui::SFML::Update(window, deltaClock.restart());
        ImGui::ShowTestWindow();

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

		pgm->lua_state.script("lol:imgui( format )");

		window.pushGLStates();
		ImGui::SFML::Render(window);
		window.popGLStates();

		window.display();


    }

    ImGui::SFML::Shutdown();

    return 0;
}
