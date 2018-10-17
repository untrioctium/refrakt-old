#include <set>
#include <map>
#include "factory.hpp"
#include "json.hpp"
#include "GLtypes.hpp"

namespace refrakt {

	struct widget: Factory<widget>
	{
		using param_t = std::map<std::string, arg_t>;
		using param_meta_t = nlohmann::json;

		widget(Key) {};
		~widget() {};

		virtual void setup(nlohmann::json config);
		virtual void run(param_t& input, param_t& output) const = 0;
		
	protected:
		std::map<std::string, param_meta_t> param_meta_;
		nlohmann::json config_;
	};
	
}
