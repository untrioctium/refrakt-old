#include <imgui.h>
#include <imgui-SFML.h>
#include <GL/glew.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <Windows.h>
#include <chrono>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <istream>
#include <fstream>
#include <iostream>

#include "log.hpp"
#include "widget.hpp"
#include "type_helpers.hpp"
#include "lua_modules.hpp"

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

		ImGui::SFML::Init(window, false);

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImFontConfig config;
		config.OversampleH = 8;
		config.OversampleV = 8;
		io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 15, &config);
		io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 13, &config);
		ImGui::SFML::UpdateFontTexture();

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}

	int run() {

		part = refrakt::widget::make("particle_widget");
		part->setup({});

		fern = refrakt::widget::make("glsl_widget");
		fern->setup({
			{"widget", {{"source", "pickover.frag"}}}
		});

		tone = refrakt::widget::make("glsl_widget");
		tone->setup({
			{ "widget",{ { "source", "tone.frag" } } }
			});

		blur = refrakt::widget::make("glsl_widget");
		blur->setup({
			{ "widget",{ { "source", "density.frag" } } }
		});

		alpha = refrakt::float_t{ 1.085f };
		max = refrakt::uint32_t{ 16 };
		sigma = refrakt::float_t{ 0.758f };
		exposure = refrakt::float_t{ 0.05f };
		preg = refrakt::float_t{ 1.22f };
		postg = refrakt::float_t{ 0.89f };
		particles = refrakt::uint32_t{ 512 };
		pick = refrakt::vec4{ -2.0f, -0.49f, -0.21f, -1.82f };
		len_pow = refrakt::float_t{ 1.0f };
		angle_pow = refrakt::float_t{ 0.17f };
		seed = refrakt::float_t{ 0.35235234230f };
		iters = 64;
		scale = refrakt::float_t{ 1.0 };
		rot = refrakt::vec3{ 0.0 };

		auto window_size = window.getSize();

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
			static sf::Clock fpsCounter;
			static float fpsAvg = 0.0;
			int lastTime = fpsCounter.restart().asMilliseconds();
			fpsAvg = .9f * fpsAvg + 100.0f / float((lastTime == 0) ? 1 : lastTime);

			ImGui::Text("Press ~ to hide guis. Press F1 to toggle fullscreen. (%.3f MB VRAM) (%.0f FPS)", refrakt::events::gl_calc_vram_usage::fire() / 1048576.0, fpsAvg);

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
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		ImGui::ShowDemoWindow();

		bool need_update = false;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("output", 0, ImGuiWindowFlags_NoScrollbar);
		auto size = ImGui::GetWindowSize();
		size.y -= ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2;
		ImGui::End();
		ImGui::PopStyleVar();

		if (!out_tex || out_tex->info().w != size.x || out_tex->info().h != size.y) {
			need_update = true;
			out_tex = pool.request(static_cast<std::size_t>( size.x ), static_cast<std::size_t>(size.y), refrakt::texture::format::Float, 4, 2);
		}
		
		ImGui::Begin("Parameters");
		need_update |= ImGui::DragInt("iters", &iters, 1.0f, 1, 64);
		need_update |= refrakt::type_helpers::imgui::display(pick, "pick", { -2.0, 2.0 }, .01f);
		need_update |= refrakt::type_helpers::imgui::display(scale, "scale", { .1, 5.0 }, .01f);
		need_update |= refrakt::type_helpers::imgui::display(rot, "rot", { -180.0, 180.0 }, 1.0f);
		need_update |= refrakt::type_helpers::imgui::display(len_pow, "len_pow", { 0.0, 3.0 }, .01f);
		need_update |= refrakt::type_helpers::imgui::display(angle_pow, "angle_pow", { 0.0, 3.0 }, .01f);
		need_update |= refrakt::type_helpers::imgui::display(particles, "sqrt(particle count)", { 16, 1024 }, 16.0f);
		need_update |= refrakt::type_helpers::imgui::display(max, "max width", { 1, 100 }, 1.0f);
		need_update |= refrakt::type_helpers::imgui::display(alpha, "alpha", { .01, 10 }, .001f);
		need_update |= refrakt::type_helpers::imgui::display(sigma, "sigma", { .01, 10 }, .001f);
		need_update |= refrakt::type_helpers::imgui::display(exposure, "exposure", { .01, 3 }, .01f);
		need_update |= refrakt::type_helpers::imgui::display(preg, "pre gamma", { .01, 3 }, .01f);
		need_update |= refrakt::type_helpers::imgui::display(postg, "post gamma", { .01, 3 }, .01f);
		ImGui::End();
		if (need_update) {
			std::uint32_t psize = std::get<refrakt::uint32_t>(particles)[0];

			auto prev_iter = pool.request(psize, psize, refrakt::texture::format::Float, 4, 4);
			auto cur_iter = pool.request(psize, psize, refrakt::texture::format::Float, 4, 4);

			auto colors = pool.request(psize, psize, refrakt::texture::format::Float, 4, 2);

			auto drawn = pool.request(static_cast<std::size_t>(size.x), static_cast<std::size_t>(size.y), refrakt::texture::format::Float, 4, 4);
			auto blurred = pool.request(static_cast<std::size_t>(size.x), static_cast<std::size_t>(size.y), refrakt::texture::format::Float, 4, 4);
			auto toned = pool.request(static_cast<std::size_t>(size.x), static_cast<std::size_t>(size.y), refrakt::texture::format::Float, 4, 4);

			refrakt::widget::param_t in, out;

			for (int i = 0; i < iters; i++) {
				in = refrakt::widget::param_t{ {"pick", pick}, {"len_pow", len_pow}, {"angle_pow", angle_pow}, {"seed", refrakt::float_t{std::sin(343.245235f * (i + 1))} }, {"last_iter", *prev_iter}, {"warmup", refrakt::int32_t{ (i % 4 == 0) ? 1u : 0u } } };
				out = refrakt::widget::param_t{ {"position", *cur_iter}, {"color", *colors} };
				fern->run(in, out);


				in = refrakt::widget::param_t{ {"pos", *cur_iter},{ "col", *colors }, {"clear", refrakt::uint32_t{ (i == 0) ? 1u : 0u }}, {"scale", scale}, {"rot", rot} };
				out = refrakt::widget::param_t{ { "result", *drawn} };
				part->run(in, out);

				std::swap(cur_iter, prev_iter);

			}

			in = refrakt::widget::param_t{ {"tex", *drawn}, {"max_width", max}, {"alpha", alpha}, {"sig", sigma} };
			out = refrakt::widget::param_t{ {"result", *blurred} };
			blur->run(in, out);

			in = refrakt::widget::param_t{ {"col", *blurred}, {"exposure", exposure}, {"pre_gamma", preg}, {"post_gamma", postg} };
			out = refrakt::widget::param_t{ {"result", *out_tex} };
			tone->run(in, out);
		}


		show_main_menu();
		window.clear();

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
		
		window.pushGLStates();
		window.resetGLStates();
		ImGui::SFML::Render(window);
		window.popGLStates();

		window.display();

		refrakt::events::gl_collect_garbage::fire();
		return true;
	}

private:
	sf::RenderWindow window;
	sf::Clock deltaClock;
	sf::ContextSettings ctx;
	sf::VideoMode screen_info;
	bool fullscreen;

	GLuint prog_;

	refrakt::texture_pool pool;
	std::unique_ptr<refrakt::widget> part, fern, tone, blur;
	refrakt::arg_t particles,alpha, max, sigma, exposure, preg, postg, pick, len_pow, angle_pow, seed, scale, rot;
	std::int32_t iters;

	refrakt::texture_handle out_tex;

};

int main(int argc, char** argv) {
	std::vector<std::string> arg;
	for (int i = 0; i < argc; i++) arg.push_back(argv[i]);

	app rfkt;
	rfkt.init(arg);
	return rfkt.run();
}
