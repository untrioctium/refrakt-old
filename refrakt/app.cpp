#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GL/glew.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <istream>
#include <fstream>
#include <iostream>

#include <GLFW/glfw3.h>

#include "log.hpp"
#include "widget.hpp"
#include "type_helpers.hpp"
#include "lua_modules.hpp"

#include <cmath>

#pragma warning(disable:4996)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#pragma warning(enable:4996)

void glfw_error(int error, const char* desc) {
	refrakt::log()->critical(desc);
}

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
	bool init(std::vector<std::string> argc) {

		int count;
		GLFWmonitor** monitors = glfwGetMonitors(&count);
		for (int i = 0; i < count; i++)
			std::cout << glfwGetMonitorName(monitors[i]) << std::endl;

		glfwSetErrorCallback(glfw_error);

		nlohmann::json settings;
		std::ifstream("settings.json") >> settings;

		if (!glfwInit()) return false;
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

		window = glfwCreateWindow(
			settings.value<int>("width", 1280),
			settings.value<int>("height", 720),
			"Refrakt", NULL, NULL
		);

		if (!window) return false;

		glfwMakeContextCurrent(window);
		glfwSwapInterval(0);

		glewExperimental = true;
		glewInit();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImFontConfig config;
		config.OversampleH = 8;
		config.OversampleV = 8;
		io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 15, &config);
		io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 13, &config);

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

		return true;
	}

	int run() {

		escape = refrakt::widget::make("glsl_widget");
		escape->setup({
			{"widget", {{"source", "escape.frag"}}}
			});

		blur = refrakt::widget::make("glsl_widget");
		blur->setup({
			{"widget", {{"source", "blur.frag"}}}
			});

		newton = refrakt::widget::make("glsl_widget");
		newton->setup({
			{"widget", {{"source", "newton.frag"}}}
			});

		setup_quad_drawer();

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		roots.push_back({ .25, .25, 1.0, 0.0 });
		roots.push_back({ -.25, .25, 1.0, 0.0 });
		roots.push_back({ .25, -.25, 1.0, 0.0 });
		roots.push_back({ -.25, -.25, 1.0, 0.0 });

		refrakt::lua::modules::load(animator.globals(), "global");
		animator.open_libraries(sol::lib::math, sol::lib::base);
		animator["center"] = refrakt::vec2{ 0.0f, 0.0f };
		animator["scale"] = refrakt::float_t{ 6.25f };
		animator["rotation"] = refrakt::float_t{ 60.0f };
		animator["hue_shift"] = refrakt::float_t{ 0.341500 };
		animator["hue_stretch"] = refrakt::float_t{ 1.0f };
		animator["exponent"] = refrakt::vec2{ 2, 0 };
		animator["escape_radius"] = refrakt::float_t{ 16 };
		animator["max_iterations"] = refrakt::uint32_t{ 17 };
		animator["julia"] = refrakt::vec2{ 1.0, 1.0 };
		animator["julia_c"] = refrakt::vec2{ 0.325000, -0.710000 };
		animator["burning_ship"] = refrakt::vec2{ 1.0, 1.0 };
		animator["hq_mode"] = refrakt::uint32_t{ 0 };
		animator["time"] = refrakt::float_t{ 0.0 };
		animator["gamma"] = refrakt::float_t{ 1.0 };
		animator["sat_mod"] = refrakt::float_t{ 1.0 };
		animator["hue_mod"] = refrakt::float_t{ 0.0 };
		animator["val_mod"] = refrakt::float_t{ 0.0 };
		animator["cloud_octaves"] = refrakt::uint32_t{ 6 };

		animator.script("function animate(t) end");

		refrakt::lua::modules::load(test_repl.globals(), "global");
		test_repl.open_libraries();

		stbi_flip_vertically_on_write(true);
		stbi_write_png_compression_level = 1;

		last_time = glfwGetTime();
		while (frame()) {}

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(window);
		glfwTerminate();

		return 0;
	}

	void setup_quad_drawer() {
		prog_ = glCreateProgram();

		GLuint vp = glCreateShader(GL_VERTEX_SHADER);
		GLuint fp = glCreateShader(GL_FRAGMENT_SHADER);

		static const char* vp_src = R"shader(
			#version 430
			out vec2 pos;
			const vec2 verts[4] = vec2[](
				vec2(-1.0f, -1.0f),
				vec2(-1.0f, 1.0f),
				vec2(1.0f, -1.0f),
				vec2(1.0f, 1.0f)
			);

			void main() {
				pos = verts[gl_VertexID] * 0.5 + 0.5;
				gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
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
	}

	void show_main_menu() {
		if (ImGui::BeginMainMenuBar()) {

			ImGui::Text("Press ~ to hide guis. Press F1 to toggle fullscreen. (%.3f MB VRAM) (%.0f FPS)",
				refrakt::events::gl_calc_vram_usage::fire() / 1048576.0, ImGui::GetIO().Framerate);

			ImGui::EndMainMenuBar();
		}
	}

	int fixed_step = false;
	double time_step = 1.0 / 24.0;
	bool paused = true;
	double time = 0;
	bool need_update = true;
	double last_time;
	bool tss = false;

	std::array<char, 1024 * 16> animator_script = { '\0' };

	void repl_window() {
		static char input[1024] = { 0 };
		static std::string log;
		ImGui::Begin("Test REPL");
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
		if (ImGui::InputText("Command", input, sizeof(input), ImGuiInputTextFlags_EnterReturnsTrue)) {
			std::string error;
			auto result = test_repl.safe_script(input, [&](lua_State*, sol::protected_function_result pfr) {
				sol::error e = pfr;
				error = e.what();
				return pfr;
			});
			log += "\n> " + std::string(input) + "\n";
			if (result.valid()) {
				log += result.operator std::string();
			}
			else log += error.substr(error.find("]:1: ") + 4);

			input[0] = 0;
			ImGui::SetKeyboardFocusHere(-1);
		}
		ImGui::Separator();
		ImGui::BeginChild("Log");

		ImGui::TextUnformatted(log.c_str());
		ImGui::PopFont();
		ImGui::EndChild();

		ImGui::End();
	}

	void animation_window() {
		ImGui::Begin("Animation");

		ImGui::LabelText("##current_time", "Current time: %.5f secs", time);
		ImGui::Text("Time Step: "); ImGui::SameLine();
		ImGui::RadioButton("Dynamic", &fixed_step, false); ImGui::SameLine();
		ImGui::RadioButton("Fixed", &fixed_step, true); ImGui::SameLine();

		if (!fixed_step)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::InputDouble("##timestep", &time_step, .01, .1, "%.10f");
		ImGui::SameLine();
		need_update |= ImGui::Checkbox("TSS (3 frames)", &tss);
		if (!fixed_step)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		static double new_time = 0.0;

		if (ImGui::Button((std::string(paused ? "Resume" : "Pause ") + "##pause").c_str())) { paused ^= true; } ImGui::SameLine();
		if (ImGui::Button("Reset to zero.")) { time = 0; need_update = true; }; ImGui::SameLine();
		if (ImGui::Button("Set to: ")) { time = new_time; need_update = true; } ImGui::SameLine();

		ImGui::InputDouble("##newtime", &new_time, .01, .1, "%.10f");
		ImGui::Separator();
		ImGui::Text("Animator");

		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
		ImGui::InputTextMultiline("##animator", animator_script.data(), sizeof(animator_script), { -1.0f, ImGui::GetTextLineHeight() * 16 }, ImGuiInputTextFlags_AllowTabInput);
		ImGui::PopFont();
		if (ImGui::Button("Update Animator")) {
			try {
				animator.safe_script("function animate(t)\n" + std::string(animator_script.data()) + "\nend");
			}
			catch (sol::error e) {
				refrakt::log()->critical(e.what());
				animator.script("function animate(t) end");
			}
			need_update = true;
		}

		ImGui::End();
	}

	void save_tex(std::string filepath, refrakt::texture_handle tex) {
		std::size_t total_size = tex->info().w * tex->info().h * tex->info().channels;
		std::vector<std::uint8_t> bytes;
		bytes.resize(total_size);
		glGetTextureImage(tex->handle(), 0, GL_RGBA, GL_UNSIGNED_BYTE, total_size, bytes.data());
		stbi_flip_vertically_on_write(true);
		stbi_write_png(filepath.c_str(), tex->info().w, tex->info().h, 4, bytes.data(), tex->info().w * 4);
	}

	int captured_frames = 0;
	bool capturing = false;
	char capture_path[256] = ".";

	void capture_window() {
		ImGui::Begin("Frame Capture");
		ImGui::LabelText("##captured", "Captured %d frames", captured_frames);
		if (ImGui::Button((std::string(capturing ? "Stop" : "Start") + " capture##togglecapture").c_str())) {
			if (capturing) captured_frames = 0;
			capturing ^= true;
		}
		ImGui::InputText("Capture path", capture_path, 256);
		ImGui::End();
	}

	void render(double t, refrakt::texture_handle tex) {
		/*if( !paused ) animator["animate"](t);

		refrakt::widget::param_t in{
			{ "center", animator["center"].get<refrakt::vec2>() },
			{ "scale", animator["scale"].get<refrakt::float_t>() },
			{ "rotation", animator["rotation"].get<refrakt::float_t>() },
			{ "hue_shift", animator["hue_shift"].get<refrakt::float_t>() },
			{ "hue_stretch", animator["hue_stretch"].get<refrakt::float_t>() },
			{ "exponent", animator["exponent"].get<refrakt::vec2>() },
			{ "escape_radius", animator["escape_radius"].get<refrakt::float_t>() },
			{ "max_iterations", animator["max_iterations"].get<refrakt::uint32_t>() },
			{ "julia", animator["julia"].get<refrakt::vec2>() },
			{ "julia_c", animator["julia_c"].get<refrakt::vec2>() },
			{ "burning_ship", animator["burning_ship"].get<refrakt::vec2>() },
			{ "hq_mode", animator["hq_mode"].get<refrakt::uint32_t>() },
			{ "surface_ratio", refrakt::float_t{float(tex->info().w) / tex->info().h } },
			{ "offset", refrakt::vec2{1.0 / tex->info().w, 1.0/ tex->info().h  }},
			{ "time", animator["time"].get<refrakt::float_t>() },
			{ "gamma", animator["gamma"].get<refrakt::float_t>() },
			{ "sat_mod", animator["sat_mod"].get<refrakt::float_t>() },
			{ "hue_mod", animator["hue_mod"].get<refrakt::float_t>() },
			{ "val_mod", animator["val_mod"].get<refrakt::float_t>() },
			{ "cloud_octaves", animator["cloud_octaves"].get<refrakt::uint32_t>() }
		};

		refrakt::widget::param_t out{
			{"color", *tex}
		};

		escape->run(in, out);*/

		refrakt::widget::param_t in{
			{"roots", roots},
			{"eccentricity", eccentricity}
		};

		refrakt::widget::param_t out{
			{"color", *tex}
		};

		newton->run(in, out);
		glFinish();
	}

	bool frame() {

		/*sf::Event event;
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
			}
		}*/

		need_update = false;

		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();
		//animation_window();
		//capture_window();
		repl_window();

		float frame_height = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2;

		ImGui::Begin("Output Window Options");
		if (ImGui::Button("Size to 480p")) ImGui::SetNextWindowSize({ 854, 480 + frame_height });
		ImGui::SameLine();
		if (ImGui::Button("Size to 720p")) ImGui::SetNextWindowSize({ 1280, 720 + frame_height });
		ImGui::SameLine();
		if (ImGui::Button("Size to 1080p")) ImGui::SetNextWindowSize({ 1920, 1080 + frame_height });
		if (ImGui::Button("Save as screenshot.png")) {
			save_tex("screenshot.png", out_tex);
		}
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("output", 0, ImGuiWindowFlags_NoScrollbar);
		auto size = ImGui::GetWindowSize();
		size.y -= frame_height;
		ImGui::End();
		ImGui::PopStyleVar();

		if (!out_tex || out_tex->info().w != size.x || out_tex->info().h != size.y) {
			need_update = true;
			out_tex = pool.request(static_cast<std::size_t>(size.x), static_cast<std::size_t>(size.y), refrakt::texture::format::Float, 4, 2);
		}
		auto now = glfwGetTime();
		if (!paused || need_update) {

			time += (fixed_step) ? time_step : now - last_time;
			need_update = true;
		}
		last_time = now;

		auto& center = animator["center"].get<refrakt::vec2>();
		auto& scale = animator["scale"].get<refrakt::float_t>();
		auto& rot = animator["rotation"].get<refrakt::float_t>();
		auto& exp = animator["exponent"].get<refrakt::vec2>();
		auto& jul = animator["julia_c"].get<refrakt::vec2>();

		ImGui::Begin("Parameters");
		need_update |= refrakt::type_helpers::imgui::display(roots, "roots", { -1.0, 1.0 }, .001);
		need_update |= refrakt::type_helpers::imgui::display(eccentricity, "eccentricity", { 0.01, 2.0 }, .0001);
		/*need_update |= refrakt::type_helpers::imgui::display(center, "center", refrakt::dvec2{ -5.0, 5.0 }, .001);
		need_update |= refrakt::type_helpers::imgui::display(scale, "scale", refrakt::dvec2{ .5, 1000.0 }, .05);
		need_update |= refrakt::type_helpers::imgui::display(rot, "rotation", refrakt::dvec2{ -360, 360 }, .05);
		need_update |= refrakt::type_helpers::imgui::display(animator["hue_shift"].get<refrakt::float_t>(), "hue_shift", refrakt::dvec2{ 0, 1 }, .0001);
		need_update |= refrakt::type_helpers::imgui::display(animator["hue_stretch"].get<refrakt::float_t>(), "hue_stretch", refrakt::dvec2{ 0, 4 }, .001);
		need_update |= refrakt::type_helpers::imgui::display(exp, "exponent", refrakt::dvec2{ -4.0, 4.0 }, .001);
		need_update |= refrakt::type_helpers::imgui::display(animator["escape_radius"].get<refrakt::float_t>(), "escape_radius", refrakt::dvec2{ 0, 100.0 }, .001);
		need_update |= refrakt::type_helpers::imgui::display(animator["max_iterations"].get<refrakt::uint32_t>(), "escape", refrakt::dvec2{ 0, 1000.0 }, 1);
		need_update |= refrakt::type_helpers::imgui::display(animator["julia"].get<refrakt::vec2>(), "julia", refrakt::dvec2{ 0, 1 }, 1);
		need_update |= refrakt::type_helpers::imgui::display(jul, "julia_c", refrakt::dvec2{ -2, 2 }, .005);
		need_update |= refrakt::type_helpers::imgui::display(animator["burning_ship"].get<refrakt::vec2>(), "burning_ship", refrakt::dvec2{ 0, 1 }, 1);
		need_update |= refrakt::type_helpers::imgui::display(animator["hq_mode"].get<refrakt::uint32_t>(), "hq_mode", refrakt::dvec2{ 0, 1 }, 1);
		need_update |= refrakt::type_helpers::imgui::display(animator["gamma"].get<refrakt::float_t>(), "gamma", refrakt::dvec2{ 0, 3 }, .0001);
		need_update |= refrakt::type_helpers::imgui::display(animator["sat_mod"].get<refrakt::float_t>(), "sat_mod", refrakt::dvec2{ 0, 1 }, .001);
		need_update |= refrakt::type_helpers::imgui::display(animator["val_mod"].get<refrakt::float_t>(), "val_mod", refrakt::dvec2{ 0, 1 }, .001);
		need_update |= refrakt::type_helpers::imgui::display(animator["hue_mod"].get<refrakt::float_t>(), "hue_mod", refrakt::dvec2{ 0, 1 }, .001);
		need_update |= refrakt::type_helpers::imgui::display(animator["cloud_octaves"].get<refrakt::uint32_t>(), "cloud_octaves", refrakt::dvec2{ 0, 24 }, 1);
		need_update |= refrakt::type_helpers::imgui::display(animator["time"].get<refrakt::float_t>(), "time", refrakt::dvec2{ 0, 100000 }, 1);

		std::string table;
		std::string osc;
		std::string temp;
		temp = "center = vec2(" + std::to_string(center.x) + ", " + std::to_string(center.y) + ")";
		table += "\t\t" + temp + ",\n";
		osc += "\t\t\t{" + temp + ", 1},\n";

		temp = "scale = float(" + std::to_string(scale.x) + ")";
		table += "\t\t" + temp + ",\n";
		osc += "\t\t\t{" + temp + ", 1},\n";

		temp = "rotation = float(" + std::to_string(rot.x) + ")";
		table += "\t\t" + temp + ",\n";
		osc += "\t\t\t{" + temp + ", 1},\n";

		temp = "exponent = vec2(" + std::to_string(exp.x) + ", " + std::to_string(exp.y) + ")";
		table += "\t\t" + temp + ",\n";
		osc += "\t\t\t{" + temp + ", 1},\n";

		temp = "julia_c = vec2(" + std::to_string(jul.x) + ", " + std::to_string(jul.y) + ")";
		table += "\t\t" + temp + ",\n";
		osc += "\t\t\t{" + temp + ", 1}\n";

		table += "\t\tosc = {\n\t\t}\n";
		table = "\t{\n" + table + "\t}";

		ImGui::InputTextMultiline("Table", table.data(), table.size(), ImVec2(0, 0), ImGuiInputTextFlags_ReadOnly);
		*/

		ImGui::End();
		if (need_update) {


			if (tss && fixed_step) {
				auto prev = pool.request(static_cast<std::size_t>(size.x), static_cast<std::size_t>(size.y), refrakt::texture::format::Float, 4, 2);
				render(time - time_step, prev);

				auto cur = pool.request(static_cast<std::size_t>(size.x), static_cast<std::size_t>(size.y), refrakt::texture::format::Float, 4, 2);
				render(time, cur);

				auto next = pool.request(static_cast<std::size_t>(size.x), static_cast<std::size_t>(size.y), refrakt::texture::format::Float, 4, 2);
				render(time + time_step, next);

				refrakt::widget::param_t in{ {"in1", *prev},{"in2", *cur} ,{"in3", *next} };
				refrakt::widget::param_t out{ {"color", *out_tex} };
				blur->run(in, out);


			}
			else 	render(time, out_tex);
		}

		//glfwMakeContextCurrent(window);

		show_main_menu();

		//glUseProgram(prog_);

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, out_tex->handle());
		//glUniform1i(glGetUniformLocation(prog_, "tex") , 0);

		//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//glUseProgram(0);

		ImGui::Begin("output");
		GLuint image = out_tex->handle();
		ImGui::Image((void*)std::size_t{ image }, size, ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));
		ImGui::End();

		ImGui::Render();
		int display_w, display_h;
		//glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);

		if (!paused && capturing) {
			captured_frames++;
			char filename[64];
			sprintf_s(filename, "frame%05d.png", captured_frames);
			std::string final_path = capture_path + std::string("\\") + filename;
			std::size_t total_size = out_tex->info().w * out_tex->info().h * out_tex->info().channels;
			std::vector<std::uint8_t> bytes;
			bytes.resize(total_size);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			glGetTextureImage(out_tex->handle(), 0, GL_RGBA, GL_UNSIGNED_BYTE, total_size, bytes.data());
			refrakt::log()->info("Writing {}: {}x{} ({} MB)", final_path,
				out_tex->info().w, out_tex->info().h, total_size / (1024.0 * 1024.0));
			// unquestionably eventually causes resource exhausation if frame generation time < image write time
			std::thread([tex = *out_tex, path = final_path, d = bytes]() {
				stbi_write_png(path.c_str(), tex.info().w, tex.info().h, 4, d.data(), tex.info().w * 4);
			}).detach();
		}

		refrakt::events::gl_collect_garbage::fire();
		return !glfwWindowShouldClose(window);
	}

private:
	GLFWwindow* window;
	bool fullscreen;

	GLuint prog_;

	refrakt::texture_pool pool;
	std::unique_ptr<refrakt::widget> escape, blur, newton;

	refrakt::vec4_array roots = refrakt::vec4_array(16);
	refrakt::float_t eccentricity = refrakt::float_t(1.0);
	sol::state animator;

	sol::state test_repl;

	refrakt::texture_handle out_tex;

};

int main(int argc, char** argv) {
	std::vector<std::string> arg;
	for (int i = 0; i < argc; i++) arg.push_back(argv[i]);

	if (arg.size() > 1 && arg[1] == "repl") {
		sol::state repl;
		refrakt::lua::modules::load(repl.globals(), "global");
		repl.open_libraries();

		std::string input;

		while (true) {
			std::cout << "> ";
			std::getline(std::cin, input);

			if (input == "exit") return 0;

			std::string error;
			auto result = repl.safe_script(input, [&](lua_State*, sol::protected_function_result pfr) {
				sol::error e = pfr;
				error = e.what();
				return pfr;
			});
			if (result.valid()) {
				std::cout << result.operator std::string();
			}
			else std::cout << error.substr(error.find("]:1: ") + 4);
			std::cout << std::endl;
		}
	}

	app rfkt;
	if (!rfkt.init(arg)) {
		refrakt::log()->critical("Initialization failed.");
		std::cin.get();
		return 1;
	}
	return rfkt.run();
}
