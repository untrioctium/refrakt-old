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

int main( int argc, char** argv )
{
	bool midiEnabled = true;
	/*midi stuff*/

	// Create an api map.
	std::map<int, std::string> apiMap;
	apiMap[RtMidi::MACOSX_CORE] = "OS-X CoreMidi";
	apiMap[RtMidi::WINDOWS_MM] = "Windows MultiMedia";
	apiMap[RtMidi::UNIX_JACK] = "Jack Client";
	apiMap[RtMidi::LINUX_ALSA] = "Linux ALSA";
	apiMap[RtMidi::RTMIDI_DUMMY] = "RtMidi Dummy";

	std::vector< RtMidi::Api > apis;
	RtMidi::getCompiledApi(apis);

	std::cout << "\nCompiled APIs:\n";
	for (unsigned int i = 0; i<apis.size(); i++)
		std::cout << "  " << apiMap[apis[i]] << std::endl;

	RtMidiIn  *midiin = 0;
	RtMidiOut *midiout = 0;

	try {

		// RtMidiIn constructor ... exception possible
		midiin = new RtMidiIn();

		std::cout << "\nCurrent input API: " << apiMap[midiin->getCurrentApi()] << std::endl;

		// Check inputs.
		unsigned int nPorts = midiin->getPortCount();
		std::cout << "\nThere are " << nPorts << " MIDI input sources available.\n";

		for (unsigned i = 0; i<nPorts; i++) {
			std::string portName = midiin->getPortName(i);
			std::cout << "  Input Port #" << i + 1 << ": " << portName << '\n';
		}

		// RtMidiOut constructor ... exception possible
		midiout = new RtMidiOut();

		std::cout << "\nCurrent output API: " << apiMap[midiout->getCurrentApi()] << std::endl;

		// Check outputs.
		nPorts = midiout->getPortCount();
		std::cout << "\nThere are " << nPorts << " MIDI output ports available.\n";

		for (unsigned i = 0; i<nPorts; i++) {
			std::string portName = midiout->getPortName(i);
			std::cout << "  Output Port #" << i + 1 << ": " << portName << std::endl;
		}
		std::cout << std::endl;

	}
	catch (RtMidiError &error) {
		error.printMessage();
	}

	std::vector<uint8_t> message;
	int nBytes, i;
	double stamp;
	// Check available ports.
	unsigned int nPorts = midiin->getPortCount();
	if (nPorts == 0) {
		std::cout << "No ports available!\n";
	}
	midiin->openPort(0);

	/*midi stuff*/
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
	pgm->lua_state.script("format = {name = 'lol', bounds={min=0.0, max=5.0}, tool_tip='this is a dang tool tip'}");

    char cmd_buf[256] = {'\0'};

	int frame = 0;

	std::cout << sizeof(dvec2) << std::endl;

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

		int processed = 0;
		while(true) {
			stamp = midiin->getMessage(&message);
			nBytes = message.size();

			if (nBytes == 0) break;
			interpret_midi(message, stamp);

			processed++;
		}
		if (processed > 0) std::cout << "Processed " << processed << " MIDI input events this frame." << std::endl;

		window.clear();

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

		pgm->lua_state.script("lol:gui( format )");

		window.pushGLStates();
		ImGui::SFML::Render(window);
		window.popGLStates();

		window.display();


    }

    ImGui::SFML::Shutdown();

    return 0;
}
