#include <cstdint>
#include <map>
#include <memory>
#include <functional>
#include <vector>

#include "global_events.hpp"

namespace refrakt {

	struct texture: public events::gl_calc_vram_usage::observer {
		enum class format {
			Float,
			SignedInt,
			UnsignedInt
		};

		struct descriptor {
			std::size_t w;
			std::size_t h;
			
			texture::format format;
			std::uint8_t channels;
			std::uint8_t bytes_per_channel;

			bool operator<(const descriptor& r) const {
				return std::make_tuple(w, h, format, channels, bytes_per_channel) < std::make_tuple(r.w, r.h, r.format, r.channels, r.bytes_per_channel);
			}

			std::size_t size() const;
		};

		texture(descriptor desc, std::uint32_t handle) : handle_(handle), info_(desc) {}

		const descriptor& info();
		std::uint32_t handle();

		void on_notify(events::gl_calc_vram_usage::tag, std::size_t& count);
	private:
		std::uint32_t handle_;
		descriptor info_;
	};

	using texture_handle = std::shared_ptr<texture>;

	class texture_pool: public events::gl_calc_vram_usage::observer {
	public:

		texture_handle request(texture::descriptor desc);
		texture_handle request(std::size_t w, std::size_t h, texture::format format, std::uint8_t channels, std::uint8_t bytes_per_channel);

		void on_notify(events::gl_calc_vram_usage::tag, std::size_t& count);

	private:
		std::map<texture::descriptor, std::vector<std::uint32_t>> pool;
	};
}