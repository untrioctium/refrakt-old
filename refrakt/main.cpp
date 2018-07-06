#include "imgui.h"
#include "imgui-SFML.h"
#include <GL/glew.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <iostream>
#include <string>
#include <fstream>

#include "GLtypes.hpp"
#include "type_helpers.hpp"
#include "TextEditor.h"
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
	void initialize(const std::string& source) {
		GLuint handle = glCreateProgram();
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

		const char* src = source.c_str();

		glShaderSource(vp, 2, vpSrc, NULL);
		glShaderSource(fp, 1, &src, NULL);

		glCompileShader(vp);
		int rvalue;
		glGetShaderiv(vp, GL_COMPILE_STATUS, &rvalue);
		if (!rvalue) {
			fprintf(stderr, "Error in compiling vp\n");
			exit(30);
		}
		glAttachShader(handle, vp);

		glCompileShader(fp);
		glGetShaderiv(fp, GL_COMPILE_STATUS, &rvalue);
		if (!rvalue) {
			GLchar log[10240];
			GLsizei length;
			glGetShaderInfoLog(fp, 10239, &length, log);
			throw refrakt::widget::compile_exception(log);
		}

		glAttachShader(handle, fp);

		glBindFragDataLocation(handle, 0, "color");
		glLinkProgram(handle);

		glGetProgramiv(handle, GL_LINK_STATUS, &rvalue);
		if (!rvalue) {
			GLchar log[10240];
			GLsizei length;
			glGetProgramInfoLog(render_prog_, 10239, &length, log);
			throw refrakt::widget::compile_exception(log);

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
		GLint posPtr = glGetAttribLocation(handle, "pos");
		glVertexAttribPointer(posPtr, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(posPtr);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		render_prog_ = handle;
	}

	auto create_parameter_set() -> refrakt::widget::parameter_set {
		return refrakt::widget::parameter_set
		{
			/*{"center", refrakt::vec2{}},
			{"scale", refrakt::float_t{.75} },
			{"hue", refrakt::struct_t{
				{"shift", refrakt::float_t{ 0.200f }},
				{"stretch", refrakt::float_t{1.0} }
			}},
			{"exponent", refrakt::vec2{ 1.9029998779296875, 0.8349999785423279 } },
			{ "escape_radius", refrakt::float_t{40000.0} },
			{ "max_iterations", refrakt::uint32_t{300} },
			{ "julia", refrakt::vec2{1.0, 1.0} },
			{ "julia_c", refrakt::vec2{ 0.29200002551078796, -0.04599998891353607 } },
			{ "burning_ship", refrakt::vec2{1.0, 1.0} },
			{ "hq_mode", refrakt::uint32_t{0} },
			{ "surface_ratio", refrakt::float_t{} },
			{ "offset", refrakt::vec2{} },
			{ "time", refrakt::float_t{} }*/

			{ "center", refrakt::vec2{-0.016f, 1.006f} },
		{ "scale", refrakt::float_t{ 56.05f } },
		{ "rotation", refrakt::float_t{ 0.0f } },
		{ "hue", refrakt::struct_t{
			{ "shift", refrakt::float_t{ 0.505300f } },
		{ "stretch", refrakt::float_t{ 2.207f } }
		} },
		{ "exponent", refrakt::vec2{ 2, 0 } },
		{ "escape_radius", refrakt::float_t{ 16 } },
		{ "max_iterations", refrakt::uint32_t{ 100 } },
		{ "julia", refrakt::vec2{ 0.0, 0.0 } },
		{ "julia_c", refrakt::vec2{ 0.29200002551078796, -0.04599998891353607 } },
		{ "burning_ship", refrakt::vec2{ 0.0, 0.0 } },
		{ "hq_mode", refrakt::uint32_t{ 0 } },
		{ "surface_ratio", refrakt::float_t{} },
		{ "offset", refrakt::vec2{} },
		{ "time", refrakt::float_t{0.0} },
		{ "gamma", refrakt::float_t{1.0} },
		{ "sat_mod", refrakt::float_t{ 1.0 } },
		{ "hue_mod", refrakt::float_t{ 1.0 } },
		{ "val_mod", refrakt::float_t{ 1.0 } },
		{ "cloud_octaves", refrakt::uint32_t{ 6 } }
		};
	}

	auto operator()(const refrakt::widget::parameter_set& p) -> refrakt::widget::parameter_set {

		glUseProgram(render_prog_);
		checkErrors("use program");
		glBindVertexArray(vert_array_);
		checkErrors("bind array");
		glBindBuffer(GL_ARRAY_BUFFER, pos_buf_);
		checkErrors("bind buffer");
		for (auto&& kv : p) {
			refrakt::type_helpers::opengl::push(render_prog_, kv.first, kv.second);
		}

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		checkErrors("draw array");
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);


		return {};
	}

	bool validate(const refrakt::widget::parameter_set &) { return true; }

	auto parameter_info(const std::string& name) -> refrakt::widget::parameter_meta {
		static std::map<std::string, refrakt::widget::parameter_meta> meta{
			{"center", { 
				"Camera center", 
				"Center point of the viewing window", 
				{-5.0, 5.0}, 
				.001f
			}},
			{ "scale",{ "Picture Scale", "Picture scale, where 2 means that the vertical length is 1/2 units",{ -5.0, 5.0 }, .001f } }
		};

		return meta[name];
	}

private:
	GLuint render_prog_;
	GLuint vert_array_;
	GLuint pos_buf_;
};

void imgui_experiment_window() {
	ImGui::Begin("Testing");
	ImGui::Text("Just a test");
	
	static const std::size_t total_samples = 100;
	static sf::Clock clock;
	static std::vector<float> frame_times(total_samples);;

	frame_times.push_back(static_cast<float>(clock.restart().asMicroseconds())/1000.0);
	frame_times.erase(frame_times.begin());

	ImGui::PlotLines("Frame times", frame_times.data(), frame_times.size(), 0, 0, 0, 100);
	
	static float s;
	ImGui::SliderFloat("##slide", &s, 0, 1);
	if (ImGui::BeginPopupContextItem()) {
		ImGui::TextDisabled("Options");
		ImGui::Separator();
		if (ImGui::Selectable("Modify twiddle")) ImGui::OpenPopup("Delete?");
		if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("All those beautiful files will be deleted.\nThis operation cannot be undone!\n\n");
			ImGui::Separator();

			//static int dummy_i = 0;
			//ImGui::Combo("Combo", &dummy_i, "Delete\0Delete harder\0");

			static bool dont_ask_me_next_time = false;
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
			ImGui::PopStyleVar();

			if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}


		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("I am a tooltip");

		ImGui::Separator();
		if (ImGui::Selectable("Set bounds")) std::cout << "lel" << std::endl;
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	ImGui::Text("Slider");
	ImGui::Separator();
	ImGui::End();
}

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

	std::ifstream t("programs\\escape.frag");
	std::string fpSrc((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());


	EscapeWidget w;
	w.initialize(fpSrc);

	auto param = w.create_parameter_set();
	auto j = nlohmann::json(param);
	for (auto&& v : j["julia:ivec2"]) std::cout << v << std::endl;

	bool showGui = true;

	sf::Clock deltaClock;
	sf::Clock timer;

	TextEditor editor;
	auto lang = TextEditor::LanguageDefinition::GLSL();
	TextEditor::Identifier id;
	id.mDeclaration = std::string("two float vector");
	lang.mIdentifiers.insert({ "vec2", id });
	editor.SetLanguageDefinition(lang);

	auto add = [](auto a, auto b) { return a + b; };
	std::cout << add(3.0, 2) << std::endl;

	editor.SetText(fpSrc);

	std::cout << sizeof(refrakt::struct_t) << std::endl;
	std::cout << sizeof(refrakt::arg_t) << std::endl;
	float time = 0;
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
				if (fullscreen)
					window.create(screenInfo, "refrakt", sf::Style::Default, ctxSettings);
				else
				{
					sf::Vector2 size = window.getSize();
					screenInfo.width = size.x;
					screenInfo.height = size.y;
					window.create(sf::VideoMode::getDesktopMode(), "refrakt", sf::Style::None, ctxSettings);
				}
				fullscreen ^= true;
				w.initialize(fpSrc);
			}
		}


		window.clear();
		auto size = window.getSize();

		ImGui::SFML::Update(window, deltaClock.restart());
		if (showGui)
		{
			//ImGui::ShowTestWindow();
			show_main_menu();
			//imgui_experiment_window();

			ImGui::Begin("Editor"); {
				if (ImGui::Button("Compile")) {
					try {
						w.initialize(editor.GetText());
						fpSrc = editor.GetText();
					}
					catch (refrakt::widget::compile_exception& e) {
						std::cout << e.what() << std::endl;
					}
				}
				editor.Render("TextEditor");
				
			} ImGui::End();
		}

		ImGui::Begin("Parameters");
		if (ImGui::Button("Copy Parameters")) {
			ImGui::SetClipboardText(nlohmann::json(param).dump().c_str());
		}

		ImGui::SameLine();

		if (ImGui::Button("Paste Parameters")) {
			refrakt::struct_t loaded = nlohmann::json::parse(ImGui::GetClipboardText());
			for (auto& [name, value] : loaded) {
				if (param.has(name)) param[name] = value;
			}
		}
		refrakt::type_helpers::imgui::display(param["center"], "center", refrakt::dvec2{ -5.0, 5.0 }, .001);
		refrakt::type_helpers::imgui::display(param["scale"], "scale", refrakt::dvec2{ .5, 1000.0 }, .05);
		refrakt::type_helpers::imgui::display(param["rotation"], "rotation", refrakt::dvec2{ -360, 360 }, .05);
		refrakt::type_helpers::imgui::display(param.get<refrakt::struct_t>("hue").get("shift"), "hue_shift", refrakt::dvec2{ 0, 1 }, .0001);
		refrakt::type_helpers::imgui::display(param.get<refrakt::struct_t>("hue").get("stretch"), "hue_stretch", refrakt::dvec2{ 0, 4 }, .001);
		refrakt::type_helpers::imgui::display(param["exponent"], "exponent", refrakt::dvec2{ -4.0, 4.0 }, .001);
		refrakt::type_helpers::imgui::display(param["escape_radius"], "escape_radius", refrakt::dvec2{ 0, 100.0 }, .001);
		refrakt::type_helpers::imgui::display(param["max_iterations"], "escape", refrakt::dvec2{ 0, 1000.0 }, 1);
		refrakt::type_helpers::imgui::display(param["julia"], "julia", refrakt::dvec2{ 0, 1 }, 1);
		refrakt::type_helpers::imgui::display(param["julia_c"], "julia_c", refrakt::dvec2{ -2, 2 }, .005);
		refrakt::type_helpers::imgui::display(param["burning_ship"], "burning_ship", refrakt::dvec2{ 0, 1 }, 1);
		refrakt::type_helpers::imgui::display(param["hq_mode"], "hq_mode", refrakt::dvec2{ 0, 1 }, 1);
		refrakt::type_helpers::imgui::display(param["gamma"], "gamma", refrakt::dvec2{ 0, 3 }, .0001);
		refrakt::type_helpers::imgui::display(param["sat_mod"], "sat_mod", refrakt::dvec2{ 0, 1 }, .001);
		refrakt::type_helpers::imgui::display(param["val_mod"], "val_mod", refrakt::dvec2{ 0, 1 }, .001);
		refrakt::type_helpers::imgui::display(param["hue_mod"], "hue_mod", refrakt::dvec2{ 0, 1 }, .001);
		refrakt::type_helpers::imgui::display(param["cloud_octaves"], "cloud_octaves", refrakt::dvec2{ 0, 24 }, 1);
		refrakt::type_helpers::imgui::display(param["time"], "time", refrakt::dvec2{ 0, 100000 }, 1);
		ImGui::End();

		param["surface_ratio"] = refrakt::float_t{ float(size.x) / float(size.y) };
		param["offset"] = refrakt::vec2{ 1.0f / size.x, 1.0f / size.y };

		glViewport(0, 0, size.x, size.y);
		w(param);

		if (showGui) {
			window.pushGLStates();
			window.resetGLStates();
			ImGui::SFML::Render(window);
			window.popGLStates();

		}
		else ImGui::EndFrame();
		window.display();
		param.get<refrakt::float_t>("time")[0] += timer.restart().asMilliseconds() / 1000.0f;
	}

	ImGui::SFML::Shutdown();

	return 0;
}

