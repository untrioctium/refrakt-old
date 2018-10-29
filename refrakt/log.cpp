#include "log.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace refrakt {
	decltype(spdlog::get("")) log() {
		static auto logger = spdlog::stdout_color_mt("console");
		return logger;
	}
}