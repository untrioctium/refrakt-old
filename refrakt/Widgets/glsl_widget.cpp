#include "widget.hpp"
#include "global_events.hpp"
#include <GL/glew.h>

class glsl_widget : 
	public refrakt::widget::Registrar<glsl_widget>, 
	public refrakt::events::gl_reset::observer {
private:

public:
	void load_state() {

	}

	void setup(nlohmann::json config) {
		refrakt::widget::setup(config);
		load_state();
	}

	void on_notify(refrakt::events::gl_reset::tag) {
		load_state();
	}
};