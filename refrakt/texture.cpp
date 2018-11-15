#include <GL/glew.h>
#include <tuple>
#include <array>
#include <iostream>
#include <numeric>

#include "texture.hpp"

namespace refrakt {
	namespace detail {
		std::uint32_t allocate_texture(texture::descriptor desc) {
			static std::map<std::tuple<std::uint8_t, std::uint8_t, texture::format>, std::array<GLenum,3>> format_map = {
				{ { 1,1,texture::format::SignedInt },{ GL_R8I,GL_RED_INTEGER,GL_INT } },
				{ { 1,2,texture::format::SignedInt },{ GL_R16I,GL_RED_INTEGER,GL_INT } },
				{ { 1,4,texture::format::SignedInt },{ GL_R32I,GL_RED_INTEGER,GL_INT } },
				{ { 2,1,texture::format::SignedInt },{ GL_RG8I,GL_RG_INTEGER,GL_INT } },
				{ { 2,2,texture::format::SignedInt },{ GL_RG16I,GL_RG_INTEGER,GL_INT } },
				{ { 2,4,texture::format::SignedInt },{ GL_RG32I,GL_RG_INTEGER,GL_INT } },
				{ { 3,1,texture::format::SignedInt },{ GL_RGB8I,GL_RGB_INTEGER,GL_INT } },
				{ { 3,2,texture::format::SignedInt },{ GL_RGB16I,GL_RGB_INTEGER,GL_INT } },
				{ { 3,4,texture::format::SignedInt },{ GL_RGB32I,GL_RGB_INTEGER,GL_INT } },
				{ { 4,1,texture::format::SignedInt },{ GL_RGBA8I,GL_RGBA_INTEGER,GL_INT } },
				{ { 4,2,texture::format::SignedInt },{ GL_RGBA16I,GL_RGBA_INTEGER,GL_INT } },
				{ { 4,4,texture::format::SignedInt },{ GL_RGBA32I,GL_RGBA_INTEGER,GL_INT } },
				{ { 1,1,texture::format::UnsignedInt },{ GL_R8UI,GL_RED_INTEGER,GL_UNSIGNED_INT } },
				{ { 1,2,texture::format::UnsignedInt },{ GL_R16UI,GL_RED_INTEGER,GL_UNSIGNED_INT } },
				{ { 1,4,texture::format::UnsignedInt },{ GL_R32UI,GL_RED_INTEGER,GL_UNSIGNED_INT } },
				{ { 2,1,texture::format::UnsignedInt },{ GL_RG8UI,GL_RG_INTEGER,GL_UNSIGNED_INT } },
				{ { 2,2,texture::format::UnsignedInt },{ GL_RG16UI,GL_RG_INTEGER,GL_UNSIGNED_INT } },
				{ { 2,4,texture::format::UnsignedInt },{ GL_RG32UI,GL_RG_INTEGER,GL_UNSIGNED_INT } },
				{ { 3,1,texture::format::UnsignedInt },{ GL_RGB8UI,GL_RGB_INTEGER,GL_UNSIGNED_INT } },
				{ { 3,2,texture::format::UnsignedInt },{ GL_RGB16UI,GL_RGB_INTEGER,GL_UNSIGNED_INT } },
				{ { 3,4,texture::format::UnsignedInt },{ GL_RGB32UI,GL_RGB_INTEGER,GL_UNSIGNED_INT } },
				{ { 4,1,texture::format::UnsignedInt },{ GL_RGBA8UI,GL_RGBA_INTEGER,GL_UNSIGNED_INT } },
				{ { 4,2,texture::format::UnsignedInt },{ GL_RGBA16UI,GL_RGBA_INTEGER,GL_UNSIGNED_INT } },
				{ { 4,4,texture::format::UnsignedInt },{ GL_RGBA32UI,GL_RGBA_INTEGER,GL_UNSIGNED_INT } },
				{ { 1,2,texture::format::Float },{ GL_R16F,GL_RED,GL_FLOAT } },
				{ { 1,4,texture::format::Float },{ GL_R32F,GL_RED,GL_FLOAT } },
				{ { 2,2,texture::format::Float },{ GL_RG16F,GL_RG,GL_FLOAT } },
				{ { 2,4,texture::format::Float },{ GL_RG32F,GL_RG,GL_FLOAT } },
				{ { 3,2,texture::format::Float },{ GL_RGB16F,GL_RGB,GL_FLOAT } },
				{ { 3,4,texture::format::Float },{ GL_RGB32F,GL_RGB,GL_FLOAT } },
				{ { 4,2,texture::format::Float },{ GL_RGBA16F,GL_RGBA,GL_FLOAT } },
				{ { 4,4,texture::format::Float },{ GL_RGBA32F,GL_RGBA,GL_FLOAT } },
			};

			GLint bound_id;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound_id);

			GLuint tex;
			glGenTextures(1, &tex);

			glBindTexture(GL_TEXTURE_2D, tex);
			auto& format = format_map.at(std::make_tuple(desc.channels, desc.bytes_per_channel, desc.format));
			glTexImage2D(GL_TEXTURE_2D, 0, format[0], static_cast<GLsizei>(desc.w), static_cast<GLsizei>(desc.h), 0, format[1], format[2], 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glBindTexture(GL_TEXTURE_2D, bound_id);

			[&](auto&&...args) { (std::cout << ... << args); std::cout << std::endl; }(
				"Allocated texture ", tex, ": ", desc.w, "x", desc.h, "x", desc.bytes_per_channel * desc.channels * 8, " (", std::uint32_t(desc.channels), " ", 
				(desc.format == refrakt::texture::format::Float)? "float": (desc.format == refrakt::texture::format::SignedInt)? "int": "uint", 
				desc.bytes_per_channel * 8, " channels, ", desc.bytes_per_channel * desc.channels * desc.w * desc.h / 1048576.0, " megabytes)"
			);

			return tex;
		}

		std::set<texture_pool*>& live_pools() {
			static std::set<texture_pool*> pools;
			return pools;
		}
	}

	std::size_t texture::descriptor::size() const {
		return w * h * channels * bytes_per_channel;
	}

	auto texture::info() const -> const texture::descriptor& {
		return info_;
	}

	auto texture::handle() const -> std::uint32_t {
		return handle_;
	}
	
	/*std::size_t texture::on_notify(events::gl_calc_vram_usage::tag) {
		return info_.size();
	}*/
	
	texture_pool::texture_pool() { detail::live_pools().insert(this); }
	texture_pool::~texture_pool() { detail::live_pools().erase(this); }

	auto texture_pool::request(std::size_t w, std::size_t h, texture::format format, std::uint8_t channels, std::uint8_t bytes_per_channel) -> texture_handle {
		return request({ w, h, format, channels, bytes_per_channel });
	}

	auto texture_pool::request(texture::descriptor desc) -> texture_handle {
		// TODO: ensure channels and bpc are sane

		// silently promote 8bpc floats to 16bpc
		if (desc.format == texture::format::Float && desc.bytes_per_channel == 1) desc.bytes_per_channel++;

		decltype(pool)::iterator sub_pool_iter = pool.find(desc);

		if (sub_pool_iter == pool.end()) {
			sub_pool_iter = pool.emplace(desc, std::vector<std::pair<std::uint32_t, std::uint32_t>>{}).first;
		}
		
		auto& sub_pool = sub_pool_iter->second;

		std::uint32_t handle;
		if (sub_pool.size() == 0) handle = detail::allocate_texture(desc);
		else {
			handle = sub_pool.back().first;
			sub_pool.pop_back();
		}

		return texture_handle{
				new texture{desc, handle},
				[this, desc](texture* t) -> void {
					if (detail::live_pools().count(this)) {
						this->pool[desc].push_back({t->handle(), this->MAX_AGE});
					}
					else {
						auto handle = t->handle();
						glDeleteTextures(1, &handle);
					}
					delete t;
				}
		};
	}

	void texture_pool::on_notify(events::gl_collect_garbage::tag) {
		for (auto& sub_pool : pool) {
			sub_pool.second.erase(std::remove_if(
				sub_pool.second.begin(),
				sub_pool.second.end(),
				[](std::pair<std::uint32_t, std::uint32_t>& v) -> bool {
					if (--v.second != 0) return false;
					glDeleteTextures(1, &v.first);
					return true;
				}
			), sub_pool.second.end());
		}
	}

	std::size_t texture_pool::on_notify(events::gl_calc_vram_usage::tag) {
		return std::accumulate(pool.begin(), pool.end(), std::size_t{ 0 }, [](auto left, auto right) {
			return left + right.first.size() * right.second.size();
		});
		//for (auto& sub_pool : pool) count += sub_pool.first.size() * sub_pool.second.size();
	}
}