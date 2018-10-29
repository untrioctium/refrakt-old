#include "widget.hpp"
#include "lua_modules.hpp"

class lua_widget : public refrakt::widget::Registrar<lua_widget> {
public:
	lua_widget() {}
	~lua_widget() {}

	void setup(nlohmann::json config) {
		refrakt::lua::modules::load(state.globals(), "global");
		refrakt::lua::modules::load(state.globals(), "gl");
		state.open_libraries();
		state.script_file("widgets/glsl.lua");
		data = state.create_table();
		sol::table args = state.create_table();

		args["source"] = R"glsl(
			#version 430
			in vec2 pos;

			layout(location=0) out vec4 color;

			void main() { color = vec4( pos.x, pos.y, 0.0, 1.0); }
		)glsl";

		state["widget"]["setup"](args);
	}

	void run(refrakt::widget::param_t& input, refrakt::widget::param_t& output) const {
		state["widget"]["run"](input, output);
	}
private:
	sol::state state;
	sol::table data;
};