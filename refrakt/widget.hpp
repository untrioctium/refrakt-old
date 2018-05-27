#include <set>
#include <map>

namespace refrakt {

	class widget
	{
	public:
		using parameter_set = std::map<std::string, refrakt::arg_t>;

		virtual void initialize() = 0;
		virtual auto operator()(const parameter_set& in) -> parameter_set = 0;
		virtual bool validate(const parameter_set& in) = 0;
		virtual auto create_parameter_set() -> parameter_set = 0;
	};

}
