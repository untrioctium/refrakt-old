#include "log.hpp"
#include "lua_modules.hpp"

namespace refrakt::lua {
	modules::module_store& modules::store() {
		static modules::module_store m;
		return m;
	}

	void modules::register_module(const std::string& name, const std::string& parent, const std::string& desc, applicator a) {
		modules::store()[parent][name] = a;

		refrakt::log()->info("Registered Lua module {}.{}: {}", parent, name, desc);
	}

	bool modules::load(sol::table table, std::string name) {
		auto period_pos = name.find('.');
		std::string mod, submod;
		if (period_pos == std::string::npos) mod = name;
		else {
			mod = name.substr(0, period_pos);
			submod = name.substr(period_pos + 1);
		}

		if (store().count(mod) == 0) return false;

		sol::table mod_table = 
			name == "global"
				? table
				: table[mod].valid()
					? table[mod]
					: table[mod] = table.create();

		if (submod.length() == 0) {
			for (auto& kv : store()[mod]) { kv.second(mod_table); }
			return true;
		}
	}
}