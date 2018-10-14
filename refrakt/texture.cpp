#include <GL/glew.h>
#include "texture.hpp"
#include <tuple>
#include <array>

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
			glTexImage2D(GL_TEXTURE_2D, 0, format[0], desc.w, desc.h, 0, format[1], format[2], 0);

			glBindTexture(GL_TEXTURE_2D, bound_id);

			return tex;
		}
	}

	std::size_t texture::descriptor::size() const {
		return w * h * channels * bytes_per_channel;
	}

	auto texture::info() -> const texture::descriptor& {
		return info_;
	}

	auto texture::handle() -> std::uint32_t {
		return handle_;
	}

	void texture::on_notify(events::gl_reset::tag) {
		handle_ = detail::allocate_texture(info_);
	}
	
	void texture::on_notify(events::gl_calc_vram_usage::tag, std::size_t& count) {
		count += info_.size();
	}
	
	auto texture_pool::request(texture::descriptor desc) -> texture_pool::texture_handle {
		// TODO: ensure channels and bpc are sane

		// silently promote 8bpc floats to 16bpc
		if (desc.format == texture::format::Float && desc.bytes_per_channel == 1) desc.bytes_per_channel++;

		decltype(pool)::iterator sub_pool_iter = pool.find(desc);

		if (sub_pool_iter == pool.end()) {
			sub_pool_iter = pool.emplace(desc, std::vector<std::uint32_t>{}).first;
		}
		
		auto& sub_pool = sub_pool_iter->second;

		if (sub_pool.size() == 0) sub_pool.push_back(detail::allocate_texture(desc));

		auto handle = sub_pool.back();
		sub_pool.pop_back();

		return texture_handle{
				new texture{desc, handle},
				[&sub_pool](texture* t) -> void {
					sub_pool.push_back(t->handle());
					delete t;
				}
		};
	}

	void texture_pool::on_notify(events::gl_reset::tag) {
		for (auto& sub_pool : pool) sub_pool.second.clear();
	}

	void texture_pool::on_notify(events::gl_calc_vram_usage::tag, std::size_t& count) {
		for (auto& sub_pool : pool) count += sub_pool.first.size() * sub_pool.second.size();
	}
}