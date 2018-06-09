#include <set>
#include <map>

namespace refrakt {

	class widget
	{
	public:
		using parameter_set = refrakt::struct_t;

		struct parameter_meta {
			std::string name;
			std::string description;
			refrakt::dvec2 bounds;
			std::float_t speed;
		};

		virtual void initialize( const std::string& src ) = 0;
		virtual auto operator()(const parameter_set& in) -> parameter_set = 0;
		virtual bool validate(const parameter_set& in) = 0;
		virtual auto create_parameter_set() -> parameter_set = 0;

		virtual auto parameter_info(const std::string& name)->parameter_meta = 0;

		class compile_exception : public std::runtime_error {
			using std::runtime_error::runtime_error;
		};
	};

}
