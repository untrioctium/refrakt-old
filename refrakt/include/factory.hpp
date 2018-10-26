// Adapted from "Unforgettable Factory Registration" by Nir Friedman
// http://www.nirfriedman.com/2018/04/29/unforgettable-factory/

#include <memory>
#include <unordered_map>
#include <iostream>
template <class Base, class... Args> class Factory {
public:

	template <class ... T>
	static std::unique_ptr<Base> make(const std::string &s, T&&... args) {
		return data().at(s)(std::forward<T>(args)...);
	}

	template <class T> struct Registrar : Base {
		friend T;

		static std::string demangle() {
			// TODO: Compiler independence, only works for MSVC right now
			for (const char* name = typeid(T).name(); *name != '\0'; name++)
				if (*name == ' ') return std::string(name + 1);

			return typeid(T).name();
		}

		static bool registerT() {
			const auto name = demangle();
			Factory::data()[name] = [](Args... args) -> std::unique_ptr<Base> {
				return std::make_unique<T>(std::forward<Args>(args)...);
			};
			return true;
		}
		static bool registered;

	private:
		Registrar() : Base(Key{}) { (void)registered; }
	};

	friend Base;

private:
	class Key {
		Key() {};
		template <class T> friend struct Registrar;
	};
	using FuncType = std::unique_ptr<Base>(*)(Args...);
	Factory() = default;

	static auto &data() {
		static std::unordered_map<std::string, FuncType> s;
		return s;
	}
};

template <class Base, class... Args>
template <class T>
bool Factory<Base, Args...>::template Registrar<T>::registered =
Factory<Base, Args...>::template Registrar<T>::registerT();