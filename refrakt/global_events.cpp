#include <numeric>
#include <vector>

#include "global_events.hpp"

namespace refrakt {
	std::size_t events::gl_calc_vram_usage::accumulate(std::vector<std::size_t> v) {
		return std::accumulate(v.begin(), v.end(), std::size_t{ 0 });
	}
}