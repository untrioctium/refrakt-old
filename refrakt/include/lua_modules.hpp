#include <sol/sol.hpp>
#include <functional>
#include <iostream>
#include <map>

namespace refrakt::lua {
	class modules {
	public:

		using applicator = std::function<void(sol::table)>;

		template <class T> struct registrar {
			static bool register_parent() {
				register_module(T::name, T::parent, T::description, T::apply);
				return true;
			}

			static bool registered;
		protected:
			registrar() { (void)registered; }
		};

		struct description {
			std::string name, parent, description;
		};

		static void register_module(const std::string& name, const std::string& parent, const std::string& desc, applicator a);

		static bool load(sol::table table, std::string name);
	private:
		using module_store = std::map<std::string, std::map<std::string, applicator>>;
		static module_store& store();
	};

	template <class T>
	bool modules::template registrar<T>::registered = modules::template registrar<T>::register_parent();
}