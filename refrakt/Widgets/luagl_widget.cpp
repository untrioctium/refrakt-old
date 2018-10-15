#include "widget.hpp"
#include "sol.hpp"

class luagl_widget : public refrakt::widget::Registrar<luagl_widget> {
private:
	sol::state state_;

private:
	void run(refrakt::widget::param_t& input, refrakt::widget::param_t& output) const {
	}
};