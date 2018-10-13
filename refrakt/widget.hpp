#include <set>
#include <map>
#include "factory.hpp"
#include "json.hpp"

namespace refrakt {

	struct widget: Factory<widget>
	{
		widget(Key) {};
	};

}
